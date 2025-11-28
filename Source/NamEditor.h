#pragma once

// clang-format off
#include "PluginProcessor.h"
#include "MyLookAndFeel.h"
#include "AssetManager.h"
#include "TopBarComponent.h"
// clang-format on

#define NUM_SLIDERS 7

class NamEditor : public juce::AudioProcessorEditor,
                  public juce::Timer,
                  public juce::Slider::Listener {
public:
  NamEditor(NamJUCEAudioProcessor &);
  ~NamEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

  void timerCallback();
  void sliderValueChanged(juce::Slider *slider);

  void setMeterPosition(bool isOnMainScreen);

  enum PluginKnobs {
    Input = 0,
    NoiseGate,
    Bass,
    Middle,
    Treble,
    Output,
    Doubler
  };

private:
  std::unique_ptr<CustomSlider> sliders[NUM_SLIDERS];
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      sliderAttachments[NUM_SLIDERS];

  juce::String sliderIDs[NUM_SLIDERS]{"INPUT_ID",  "NGATE_ID",  "BASS_ID",
                                      "MIDDLE_ID", "TREBLE_ID", "OUTPUT_ID",
                                      "DOUBLER_ID"};

  std::unique_ptr<AssetManager> assetManager;

  // juce::TooltipWindow tooltipWindow{ this, 200 };

  knobLookAndFeel lnf{knobLookAndFeel::KnobTypes::Main};

  juce::String ngThreshold{"Null"};

  int screensOffset = 46;

  foleys::LevelMeter meterIn{foleys::LevelMeter::SingleChannel},
      meterOut{foleys::LevelMeter::SingleChannel};
  MeterLookAndFeel meterlnf, meterlnf2;

  TopBarComponent topBar;
  juce::Label debugLabel;

  NamJUCEAudioProcessor &audioProcessor;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NamEditor)

  // Private Functions
private:
  // Pass this to the Preset Manager for updating the gui after loading a new
  // preset. Maybe not the best way of doing it...
  void updateAfterPresetLoad();
};