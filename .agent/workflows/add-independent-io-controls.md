---
description: Implementation plan for adding independent input/output controls
---

# Implementation Plan: Independent Input/Output Controls

## Overview
Add two new parameters (Independent Input Gain and Independent Output Gain) that are separate from the NAM processor's internal input/output controls. This provides better gain staging and matches the Neural DSP workflow.

## Signal Flow (After Implementation)
```
Input Signal 
→ [NEW] Independent Input Gain
→ Input Meter
→ Noise Gate
→ NAM Input Gain (existing, part of NAM model)
→ NAM Model Processing
→ Tone Stack
→ NAM Output Gain (existing, part of NAM model)
→ [NEW] Independent Output Gain
→ Doubler
→ Output Meter
```

---

## File Modifications Required

### 1. **PluginProcessor.h**
**Location:** `/Users/timpelser/Documents/SideProjects/Mayerism/codebase/nam-juce/Source/PluginProcessor.h`

**Changes:**
- Add two new member variables to store parameter pointers:
  ```cpp
  std::atomic<float>* pluginInputGain;
  std::atomic<float>* pluginOutputGain;
  ```

**Why:** Need to store references to the new parameters for use in processBlock.

---

### 2. **PluginProcessor.cpp**
**Location:** `/Users/timpelser/Documents/SideProjects/Mayerism/codebase/nam-juce/Source/PluginProcessor.cpp`

#### **Change A: createParameters() method (around line 200-213)**
**Add two new parameters:**
```cpp
// After NAM parameters, before DOUBLER_SPREAD_ID
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "PLUGIN_INPUT_ID", "PLUGIN_INPUT", -20.0f, 20.0f, 0.0f));
parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
    "PLUGIN_OUTPUT_ID", "PLUGIN_OUTPUT", -40.0f, 40.0f, 0.0f));
```

**Why:** Creates the audio parameters in the plugin's parameter tree.

#### **Change B: prepareToPlay() method (around line 89)**
**Hook the parameter pointers:**
```cpp
// After myNAM.hookParameters(apvts);
pluginInputGain = apvts.getRawParameterValue("PLUGIN_INPUT_ID");
pluginOutputGain = apvts.getRawParameterValue("PLUGIN_OUTPUT_ID");
```

**Why:** Gets direct access to parameter values for efficient real-time processing.

#### **Change C: processBlock() method (around line 157-185)**
**Current flow:**
```cpp
Line 159: meterInSource.measureBlock(buffer);
Line 172: myNAM.processBlock(buffer);
Line 179-182: // Doubler processing
Line 184: meterOutSource.measureBlock(buffer);
```

**New flow:**
```cpp
// Apply independent input gain BEFORE input metering
buffer.applyGain(dB_to_linear(pluginInputGain->load()));

meterInSource.measureBlock(buffer);

// ... existing code (NAM processing, doubler) ...

// Apply independent output gain AFTER doubler, BEFORE output metering
buffer.applyGain(dB_to_linear(pluginOutputGain->load()));

meterOutSource.measureBlock(buffer);
```

**Add helper method:**
```cpp
// In private section or as inline
double dB_to_linear(double db_value) {
  return std::pow(10.0, db_value / 20.0);
}
```

**Why:** 
- Input gain affects what level hits the amp (and meters show post-input-gain level)
- Output gain is the final level control (meters show final output level)

---

### 3. **NamEditor.h**
**Location:** `/Users/timpelser/Documents/SideProjects/Mayerism/codebase/nam-juce/Source/NamEditor.h`

#### **Change A: Update NUM_SLIDERS define (line 10)**
```cpp
#define NUM_SLIDERS 9  // Changed from 7
```

**Why:** We're adding 2 more sliders.

#### **Change B: Update PluginKnobs enum (lines 27-35)**
```cpp
enum PluginKnobs {
  PluginInput = 0,    // NEW - Independent input
  Input,              // NAM amp input (was 0, now 1)
  NoiseGate,          // (was 1, now 2)
  Bass,               // (was 2, now 3)
  Middle,             // (was 3, now 4)
  Treble,             // (was 4, now 5)
  Output,             // NAM amp output (was 5, now 6)
  PluginOutput,       // NEW - Independent output
  Doubler             // (was 6, now 8)
};
```

**Why:** Adds the two new knobs to the enum, maintaining logical signal flow order.

#### **Change C: Update sliderIDs array (lines 42-44)**
```cpp
juce::String sliderIDs[NUM_SLIDERS]{
    "PLUGIN_INPUT_ID",  // NEW
    "INPUT_ID", 
    "NGATE_ID", 
    "BASS_ID",
    "MIDDLE_ID", 
    "TREBLE_ID", 
    "OUTPUT_ID",
    "PLUGIN_OUTPUT_ID", // NEW
    "DOUBLER_ID"
};
```

**Why:** Maps UI sliders to parameter IDs in APVTS.

---

### 4. **NamEditor.cpp**
**Location:** `/Users/timpelser/Documents/SideProjects/Mayerism/codebase/nam-juce/Source/NamEditor.cpp`

#### **Change A: Update positioning logic (lines 21-48)**

**Current positioning:**
- `xStart = 266`
- `xOffsetMultiplier = 74`
- Main row knobs: Input, Bass, Middle, Treble, Output
- Special positioned: NoiseGate, Doubler

**New positioning strategy:**
You have several options:

**Option 1: Add to main row (requires UI space check)**
- Main row: PluginInput, Input, Bass, Middle, Treble, Output, PluginOutput
- Still exclude NoiseGate and Doubler from main row

**Option 2: Smart positioning**
- PluginInput: Position before/near Input meter (left side)
- PluginOutput: Position after/near Output meter (right side)
- Keep middle section as-is

**Option 3: Adjust spacing**
- Reduce `xOffsetMultiplier` slightly to fit all 7 knobs in main row
- Or adjust `xStart` to shift everything

**Recommended positioning code:**
```cpp
// Update the positioning condition to exclude NoiseGate, Doubler, and the independent controls
if (slider != PluginKnobs::NoiseGate && 
    slider != PluginKnobs::Doubler &&
    slider != PluginKnobs::PluginInput &&
    slider != PluginKnobs::PluginOutput) {
  sliders[slider]->setBounds(xStart + (positionIndex * xOffsetMultiplier),
                             450, knobSize, knobSize);
  positionIndex++;
}
```

**Then manually position the new knobs:**
```cpp
// Position PluginInput (near input meter, left side)
sliders[PluginKnobs::PluginInput]->setBounds(
    80,   // X position (adjust based on your UI)
    230,  // Y position (adjust based on your UI)
    knobSize, knobSize
);

// Position PluginOutput (near output meter, right side)
sliders[PluginKnobs::PluginOutput]->setBounds(
    getWidth() - 130,  // X position (adjust based on your UI)
    230,               // Y position (adjust based on your UI)
    knobSize, knobSize
);

// Or add them to the main row by reducing spacing:
// int xOffsetMultiplier = 66;  // Reduced from 74
// Then let them be positioned in the main row loop
```

**Why:** UI needs to accommodate the two new knobs. Exact positioning depends on your background image design.

---

## Testing Checklist

After implementation, verify:

- [ ] **Parameters created**: Check plugin parameters in DAW - should see PLUGIN_INPUT and PLUGIN_OUTPUT
- [ ] **UI renders**: Both new knobs visible and positioned correctly
- [ ] **Parameter linking**: Moving knobs changes corresponding parameter values
- [ ] **Signal flow**: 
  - Increasing PluginInput makes signal louder into amp (meters reflect this)
  - Increasing PluginOutput makes final output louder
  - NAM Input/Output still function as expected
- [ ] **Automation**: Both new parameters can be automated in DAW
- [ ] **Preset saving**: Parameters save/load with presets
- [ ] **No clipping**: Verify meters don't clip unexpectedly
- [ ] **CPU performance**: No significant CPU increase

---

## Parameter Ranges (Proposed)

- **PLUGIN_INPUT_ID**: -20.0 dB to +20.0 dB (default: 0.0 dB)
  - Enough range to accommodate different pickup outputs
  - Matches NAM's INPUT_ID range
  
- **PLUGIN_OUTPUT_ID**: -40.0 dB to +40.0 dB (default: 0.0 dB)
  - Wider range for final output matching
  - Matches NAM's OUTPUT_ID range

---

## UI Labeling Suggestions

For visual clarity, consider labeling the knobs as:

- **PluginInput** → "**TRIM**" or "**IN**" (to differentiate from NAM Input)
- **NAM Input** → "**GAIN**" or "**DRIVE**" (currently labeled "INPUT")
- **NAM Output** → "**VOLUME**" or "**MASTER**" (currently labeled "OUTPUT")
- **PluginOutput** → "**OUTPUT**" or "**OUT**" 

This makes the distinction clear to users.

---

## Estimated Implementation Time
- **Code changes**: 30-45 minutes
- **UI positioning/testing**: 15-30 minutes (depends on background image alignment needs)
- **Testing**: 15-20 minutes
- **Total**: ~1-1.5 hours

---

## Notes

1. **Meter positioning**: The current implementation shows meters AFTER processing. With independent input gain, the input meter will now show the level after the PluginInput gain is applied.

2. **Gain staging workflow**:
   - User adjusts PluginInput to match their guitar/pickup to desired level
   - User adjusts NAM Input to control amp drive/saturation
   - User adjusts NAM Output as part of amp tone
   - User adjusts PluginOutput to match final level in mix

3. **Doubler placement**: Doubler remains after all gain stages, which is correct for stereo widening.

4. **UI space**: You'll need to verify that your background image has space for 2 additional knobs, or adjust the layout accordingly.
