#pragma once
#include "dsp/ClippingStage.h"
#include "dsp/ToneStage.h"
#include <JuceHeader.h>

/**
 * TubeScreamer TS808 Processor
 * Handles the drive/clipping stage and tone control
 * Based on circuit-accurate WDF (Wave Digital Filter) implementation
 */
class TSProcessor {
public:
  TSProcessor() {}
  ~TSProcessor() {}

  void prepare(const juce::dsp::ProcessSpec &spec) {
    sampleRate = spec.sampleRate;
    maxBlockSize = spec.maximumBlockSize;
    numChannels = spec.numChannels;

    // Initialize oversampling (2x oversampling, order 1)
    oversampling.initProcessing(maxBlockSize);

    // Oversample factor for clipping stage
    const float oversampledRate =
        (float)sampleRate * std::pow(2.0f, (float)oversampleFactor);

    // Prepare clipping and tone stages for each channel
    for (int ch = 0; ch < 2; ++ch) {
      clippingStage[ch].prepare(oversampledRate);
      clippingStage[ch].setDrive(currentDrive);

      toneStage[ch].prepare((float)sampleRate);
      toneStage[ch].setTone(currentTone);
    }
  }

  void reset() {
    oversampling.reset();

    for (int ch = 0; ch < 2; ++ch) {
      clippingStage[ch].reset();
      toneStage[ch].reset();
    }
  }

  /**
   * Set the drive amount (0.0 to 10.0)
   * Controls the gain into the clipping stage
   */
  void setDrive(float drive) {
    currentDrive = juce::jlimit(0.0f, 10.0f, drive);
  }

  /**
   * Set the tone control (0.0 to 10.0)
   * 0 = dark/bass, 10 = bright/treble
   */
  void setTone(float tone) { currentTone = juce::jlimit(0.0f, 10.0f, tone); }

  /**
   * Set the level/output volume (0.0 to 10.0)
   * Controls the output volume after tone shaping
   * This is the standard TS-808 Level knob
   */
  void setLevel(float level) {
    currentLevel = juce::jlimit(0.0f, 10.0f, level);
  }

  /**
   * Process audio buffer through TS808 clipping + tone stages
   */
  void process(juce::AudioBuffer<float> &buffer) {
    const auto numSamples = buffer.getNumSamples();
    juce::dsp::AudioBlock<float> block(buffer);

    // ========== CLIPPING STAGE (with 2x oversampling) ==========
    auto osBlock = oversampling.processSamplesUp(block);

    for (int ch = 0; ch < osBlock.getNumChannels(); ++ch) {
      clippingStage[ch].setDrive(currentDrive);
      auto *x = osBlock.getChannelPointer(ch);

      for (int n = 0; n < osBlock.getNumSamples(); ++n) {
        x[n] = clippingStage[ch].processSample(x[n]);
      }
    }

    oversampling.processSamplesDown(block);

    // ========== TONE STAGE ==========
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
      auto *x = buffer.getWritePointer(ch);

      toneStage[ch].setTone(currentTone);
      toneStage[ch].processBlock(x, numSamples);
    }

    // ========== LEVEL (Output Volume) ==========
    // Simple linear gain, just like the real TS-808 Level pot
    const float levelGain = currentLevel / 10.0f;
    buffer.applyGain(levelGain);
  }

  float getCurrentDrive() const { return currentDrive; }
  float getCurrentTone() const { return currentTone; }
  float getCurrentLevel() const { return currentLevel; }

private:
  // Audio processing specs
  double sampleRate = 44100.0;
  int maxBlockSize = 512;
  int numChannels = 2;

  // TS808 DSP stages (stereo)
  ClippingStage clippingStage[2];
  ToneStage toneStage[2];

  // Oversampling for clipping stage (reduces aliasing)
  static constexpr size_t oversampleFactor = 1; // 2x oversampling
  juce::dsp::Oversampling<float> oversampling{
      2, oversampleFactor,
      juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR};

  // Current parameter values
  float currentDrive = 2.0f; // Default drive
  float currentTone = 5.0f;  // Default tone (middle position)
  float currentLevel = 7.0f; // Default level (real TS-808 unity gain ~70%)
};
