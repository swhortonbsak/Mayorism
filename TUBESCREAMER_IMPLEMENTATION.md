# 🎸 Tubescreamer Implementation Plan

## Overview
This document outlines the complete implementation of a Tubescreamer overdrive pedal effect in the NAM JUCE plugin. The implementation is split into two phases: Audio Processing (DSP) and User Interface (UI).

---

## 📐 Signal Chain Architecture

### Current Signal Chain
```
1. Input Gain (PLUGIN_INPUT_ID) 
2. Input Metering
3. NAM Amp Processing (includes: Input Level → NoiseGate → Model → ToneStack → Output Level)
4. Dual Mono conversion
5. Doubler effect
6. Output Gain (PLUGIN_OUTPUT_ID)
7. Output Metering
```

### Proposed Signal Chain (with Tubescreamer)
```
1. Input Gain (PLUGIN_INPUT_ID)
2. Input Metering
3. ✨ TUBESCREAMER (NEW) ← Pre-amp overdrive effect
4. NAM Amp Processing
5. Dual Mono conversion
6. Doubler effect
7. Output Gain (PLUGIN_OUTPUT_ID)
8. Output Metering
```

**Key Point**: The Tubescreamer is placed BEFORE the NAM amp as a pre-amp overdrive/boost pedal (authentic to real-world usage).

---

## 🎛️ Tubescreamer Parameters

| Parameter | ID | Range | Default | Description |
|-----------|-----|-------|---------|-------------|
| **Drive** | `TUBESCREAMER_DRIVE_ID` | 0.0 - 10.0 | 5.0 | Controls pre-gain/distortion amount |
| **Tone** | `TUBESCREAMER_TONE_ID` | 0.0 - 10.0 | 5.0 | Controls mid-frequency emphasis |
| **Level** | `TUBESCREAMER_LEVEL_ID` | 0.0 - 10.0 | 5.0 | Output volume compensation |
| **Enabled** | `TUBESCREAMER_ENABLED_ID` | Bool | false | On/Off bypass |

---

## 🔧 Phase 1: Audio Processing (DSP)

### Files to Create/Modify

| File | Action | Description |
|------|--------|-------------|
| `TubescreamerProcessor.h` | **CREATE** | New DSP module with Drive/Tone/Level processing |
| `PluginProcessor.h` | **MODIFY** | Add include, member variable, parameter pointers |
| `PluginProcessor.cpp` | **MODIFY** | Initialize in `prepareToPlay()`, process in `processBlock()`, add parameters in `createParameters()` |

---

### Step 1: Create `TubescreamerProcessor.h`

**Location**: `/Source/TubescreamerProcessor.h`

**Complete Code**:

```cpp
#pragma once
#include <JuceHeader.h>

// Soft clipping waveshaper for Tubescreamer distortion
class TubescreamerWaveshaper
{
public:
    // Asymmetric soft clipping using tanh
    inline float process(float input, float drive)
    {
        // Pre-gain for drive control
        float driven = input * drive;
        // Soft clipping with tanh (smooth saturation)
        return std::tanh(driven);
    }
};

class Tubescreamer
{
public:
    Tubescreamer();
    ~Tubescreamer();
    
    void prepare(juce::dsp::ProcessSpec& spec);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();
    
    // Parameter setters
    void setDrive(float newDrive);     // 0.0 - 10.0
    void setTone(float newTone);       // 0.0 - 10.0 (controls mid-hump)
    void setLevel(float newLevel);     // 0.0 - 10.0 (output volume)
    void setEnabled(bool enabled);
    
private:
    juce::dsp::ProcessSpec spec;
    
    // DSP Components
    TubescreamerWaveshaper waveshaper;
    
    // Pre-clipping high-pass filter (cuts bass before distortion)
    juce::dsp::IIR::Filter<float> preFilter;
    
    // Post-clipping tone control (mid-hump characteristic)
    juce::dsp::IIR::Filter<float> toneFilterLow;
    juce::dsp::IIR::Filter<float> toneFilterHigh;
    
    // Parameters
    float drive = 1.0f;
    float tone = 5.0f;
    float level = 5.0f;
    bool isEnabled = false;
    
    double sampleRate = 44100.0;
    
    // Helper method
    void updateToneFilters();
};

// Implementation (can be moved to .cpp if preferred)
inline Tubescreamer::Tubescreamer() {}

inline Tubescreamer::~Tubescreamer() {}

inline void Tubescreamer::prepare(juce::dsp::ProcessSpec& _spec)
{
    spec = _spec;
    sampleRate = spec.sampleRate;
    
    // Pre-clipping high-pass filter (720Hz) - prevents muddy bass distortion
    auto preCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, 720.0f, 0.707f);
    *preFilter.coefficients = *preCoeffs;
    preFilter.prepare(spec);
    
    // Initialize tone filters
    toneFilterHigh.prepare(spec);
    toneFilterLow.prepare(spec);
    updateToneFilters();
    
    reset();
}

inline void Tubescreamer::reset()
{
    preFilter.reset();
    toneFilterLow.reset();
    toneFilterHigh.reset();
}

inline void Tubescreamer::updateToneFilters()
{
    // Mid-hump characteristic around 1kHz (signature Tubescreamer sound)
    float midFreq = juce::jmap(tone, 0.0f, 10.0f, 500.0f, 2000.0f);
    
    auto toneCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, midFreq, 1.5f, juce::Decibels::decibelsToGain(6.0f));
    *toneFilterHigh.coefficients = *toneCoeffs;
}

inline void Tubescreamer::process(juce::AudioBuffer<float>& buffer)
{
    if (!isEnabled) return;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float input = channelData[sample];
            
            // 1. Pre-filter (high-pass to tighten bass)
            float filtered = preFilter.processSample(input);
            
            // 2. Soft clipping with drive
            float driven = waveshaper.process(filtered, drive);
            
            // 3. Tone shaping (mid-hump)
            float toned = toneFilterHigh.processSample(driven);
            
            // 4. Output level compensation
            channelData[sample] = toned * (level / 10.0f);
        }
    }
}

inline void Tubescreamer::setDrive(float newDrive) 
{ 
    // Map 0-10 range to 1-20 for actual drive multiplication
    drive = juce::jmap(newDrive, 0.0f, 10.0f, 1.0f, 20.0f); 
}

inline void Tubescreamer::setTone(float newTone) 
{ 
    tone = newTone; 
    updateToneFilters(); 
}

inline void Tubescreamer::setLevel(float newLevel) 
{ 
    level = newLevel; 
}

inline void Tubescreamer::setEnabled(bool enabled) 
{ 
    isEnabled = enabled; 
}
```

**DSP Explanation**:
- **Pre-filter**: High-pass at 720Hz cuts bass before clipping (prevents muddy distortion)
- **Waveshaper**: `tanh()` function for smooth, symmetrical soft clipping
- **Tone filter**: Parametric EQ boost for the classic mid-hump (500Hz - 2kHz)
- **Level**: Output gain to compensate for volume loss/boost

---

### Step 2: Modify `PluginProcessor.h`

**Add to includes** (around line 8):
```cpp
#include "DoublerProcessor.h"
#include "TubescreamerProcessor.h"  // ← ADD THIS
#include "PresetManager/PresetManager.h"
```

**Add member variable** (around line 77, in private section):
```cpp
Doubler doubler;
Tubescreamer tubescreamer;  // ← ADD THIS
```

**Add parameter pointers** (around line 87, in private section):
```cpp
// Independent input/output gain parameters
std::atomic<float>* pluginInputGain;
std::atomic<float>* pluginOutputGain;

// Tubescreamer parameters  ← ADD THIS BLOCK
std::atomic<float>* tubescreamerDrive;
std::atomic<float>* tubescreamerTone;
std::atomic<float>* tubescreamerLevel;
std::atomic<float>* tubescreamerEnabled;
```

---

### Step 3: Modify `PluginProcessor.cpp`

#### A) Update `prepareToPlay()` function

**Location**: Around line 95, after `doubler.prepare(spec);`

**Add**:
```cpp
doubler.prepare(spec);

// Prepare Tubescreamer
tubescreamer.prepare(spec);

// Hook Tubescreamer parameters
tubescreamerDrive = apvts.getRawParameterValue("TUBESCREAMER_DRIVE_ID");
tubescreamerTone = apvts.getRawParameterValue("TUBESCREAMER_TONE_ID");
tubescreamerLevel = apvts.getRawParameterValue("TUBESCREAMER_LEVEL_ID");
tubescreamerEnabled = apvts.getRawParameterValue("TUBESCREAMER_ENABLED_ID");

meterInSource.resize(getTotalNumOutputChannels(),
                     sampleRate * 0.1 / samplesPerBlock);
```

#### B) Update `processBlock()` function

**Location**: Around line 178, BEFORE `myNAM.processBlock(buffer);`

**Add**:
```cpp
meterInSource.measureBlock(buffer);

// Tubescreamer processing (before NAM amp for pre-amp overdrive)
if (tubescreamerEnabled->load() > 0.5f) {
    tubescreamer.setDrive(tubescreamerDrive->load());
    tubescreamer.setTone(tubescreamerTone->load());
    tubescreamer.setLevel(tubescreamerLevel->load());
    tubescreamer.process(buffer);
}

myNAM.processBlock(buffer);  // NAM amp processing comes after
```

#### C) Update `createParameters()` function

**Location**: Around line 225, after the Doubler parameter

**Add**:
```cpp
auto normRange = NormalisableRange<float>(0.0, 20.0, 0.1f);
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "DOUBLER_SPREAD_ID", "DOUBLER_SPREAD", normRange, 0.0));

// Tubescreamer parameters
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "TUBESCREAMER_DRIVE_ID", "TS_DRIVE", 0.0f, 10.0f, 5.0f));
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "TUBESCREAMER_TONE_ID", "TS_TONE", 0.0f, 10.0f, 5.0f));
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "TUBESCREAMER_LEVEL_ID", "TS_LEVEL", 0.0f, 10.0f, 5.0f));
parameters.push_back(std::make_unique<juce::AudioParameterBool>(
    "TUBESCREAMER_ENABLED_ID", "TS_ENABLED", false));

parameters.push_back(std::make_unique<juce::AudioParameterBool>(
    "SMALL_WINDOW_ID", "SMALL_WINDOW", false, "SMALL_WINDOW"));
```

---

## ✅ Implementation Checklist

### Phase 1: Audio Processing (Do First)
- [ ] Create `TubescreamerProcessor.h` file
- [ ] Modify `PluginProcessor.h` (include, member, parameters)
- [ ] Modify `PluginProcessor.cpp` (prepare, process, createParameters)
- [ ] Build and test audio (can use default values without UI)
- [ ] Verify Tubescreamer is in signal chain before NAM amp
- [ ] Test Drive, Tone, Level parameters programmatically

---

## 📚 References

- [Tubescreamer Circuit Analysis - ElectroSmash](https://www.electrosmash.com/tube-screamer-analysis)
- [JUCE dsp Module Documentation](https://docs.juce.com/master/group__juce__dsp.html)
- [Digital Waveshaping and Distortion](https://www.musicdsp.org/)

---

**Last Updated**: 2025-12-03  
**Status**: Ready for Phase 1 implementation
