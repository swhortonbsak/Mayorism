#pragma once
#include "BinaryData.h"
#include <JuceHeader.h>

// This will need refactoring...
class AssetManager {
public:
  AssetManager() {}

  ~AssetManager() {}

  juce::Image getBackground() { return background; }

  juce::Image getScreens() { return screens; }

  void setLoadButton(std::unique_ptr<juce::ImageButton> &button) {
    button->setImages(false, true, false, loadButtonUnpressed, 1.0,
                      juce::Colours::transparentWhite, loadButtonUnpressed, 1.0,
                      juce::Colours::transparentWhite, loadButtonPressed, 1.0,
                      juce::Colours::transparentWhite, 0);
  }

  void setClearButton(std::unique_ptr<juce::ImageButton> &button) {
    button->setImages(false, true, false, clearButtonUnpressed, 1.0,
                      juce::Colours::transparentWhite, clearButtonUnpressed,
                      1.0, juce::Colours::transparentWhite, clearButtonPressed,
                      1.0, juce::Colours::transparentWhite, 0);
  }

private:
  juce::Image background = juce::ImageFileFormat::loadFrom(
      BinaryData::background_png, BinaryData::background_pngSize);
  juce::Image screens = juce::ImageFileFormat::loadFrom(
      BinaryData::screens_png, BinaryData::screens_pngSize);

  // Load IR Button Assets
  juce::Image loadButtonPressed = juce::ImageFileFormat::loadFrom(
      BinaryData::loadButtonPushed_png, BinaryData::loadButtonPushed_pngSize);
  juce::Image loadButtonUnpressed =
      juce::ImageFileFormat::loadFrom(BinaryData::loadButtonUnpushed_png,
                                      BinaryData::loadButtonUnpushed_pngSize);

  juce::Image clearButtonPressed =
      juce::ImageFileFormat::loadFrom(BinaryData::clearButtonPressed_png,
                                      BinaryData::clearButtonPressed_pngSize);
  juce::Image clearButtonUnpressed =
      juce::ImageFileFormat::loadFrom(BinaryData::clearButtonUnpressed_png,
                                      BinaryData::clearButtonUnpressed_pngSize);
};