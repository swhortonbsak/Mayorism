#include "NamEditor.h"

NamEditor::NamEditor(NamJUCEAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      topBar(p, [&]() { updateAfterPresetLoad(); }) {
  assetManager.reset(new AssetManager());

  // Meters
  meterIn.setMeterSource(&audioProcessor.getMeterInSource());
  addAndMakeVisible(meterIn);

  meterOut.setMeterSource(&audioProcessor.getMeterOutSource());
  addAndMakeVisible(meterOut);

  meterIn.setAlpha(0.8);
  meterOut.setAlpha(0.8);

  meterIn.setSelectedChannel(0);
  meterOut.setSelectedChannel(0);

  int knobSize = 51;
  int xStart = 266;
  int xOffsetMultiplier = 74;

  lnf.setColour(Slider::textBoxOutlineColourId,
                juce::Colours::transparentBlack);
  lnf.setColour(Slider::textBoxBackgroundColourId,
                juce::Colours::transparentBlack);
  lnf.setColour(Slider::textBoxTextColourId, juce::Colours::ivory);

  // Setup sliders
  int positionIndex = 0; // Separate counter for main row positioning
  for (int slider = 0; slider < NUM_SLIDERS; ++slider) {
    sliders[slider].reset(new CustomSlider());
    addAndMakeVisible(sliders[slider].get());
    sliders[slider]->setLookAndFeel(&lnf);
    sliders[slider]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    sliders[slider]->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
    // sliders[slider]->setPopupDisplayEnabled(true, true,
    // getTopLevelComponent());

    // Position only main row knobs (skip NoiseGate and Doubler - positioned
    // manually below)
    if (slider != PluginKnobs::NoiseGate && slider != PluginKnobs::Doubler) {
      sliders[slider]->setBounds(xStart + (positionIndex * xOffsetMultiplier),
                                 450, knobSize, knobSize);
      positionIndex++;
    }
  }

  sliders[PluginKnobs::Doubler]->setPopupDisplayEnabled(true, true,
                                                        getTopLevelComponent());
  sliders[PluginKnobs::Doubler]->setCustomSlider(
      CustomSlider::SliderTypes::Doubler);
  sliders[PluginKnobs::Doubler]->setTextBoxStyle(juce::Slider::NoTextBox, false,
                                                 80, 20);
  sliders[PluginKnobs::Doubler]->setBounds(sliders[PluginKnobs::Output]->getX(),
                                           80, knobSize, knobSize);
  // Position NoiseGate independently (next to Doubler)
  sliders[PluginKnobs::NoiseGate]->setBounds(
      sliders[PluginKnobs::Output]->getX() -
          140, // 140px to the left of Doubler
      80,      // Same Y as Doubler
      knobSize, knobSize);
  sliders[PluginKnobs::NoiseGate]->setPopupDisplayEnabled(
      true, true, getTopLevelComponent());
  sliders[PluginKnobs::NoiseGate]->setCustomSlider(
      CustomSlider::SliderTypes::Gate);
  sliders[PluginKnobs::NoiseGate]->setPopupDisplayEnabled(
      true, true, getTopLevelComponent());
  sliders[PluginKnobs::NoiseGate]->addListener(this);

  // Hook slider and button attacments
  for (int slider = 0; slider < NUM_SLIDERS; ++slider)
    sliderAttachments[slider] =
        std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, sliderIDs[slider], *sliders[slider]);

  sliderAttachments[PluginKnobs::Doubler] =
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          audioProcessor.apvts, "DOUBLER_SPREAD_ID",
          *sliders[PluginKnobs::Doubler]);

  meterIn.toFront(true);
  meterOut.toFront(true);

  addAndMakeVisible(&topBar);

  addAndMakeVisible(debugLabel);
  debugLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  debugLabel.setJustificationType(juce::Justification::centred);

  startTimer(30);
}

NamEditor::~NamEditor() {
  for (int sliderAtt = 0; sliderAtt < NUM_SLIDERS; ++sliderAtt)
    sliderAttachments[sliderAtt] = nullptr;
}

void NamEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour::fromString("FF121212"));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);

  g.drawImageAt(assetManager->getBackground(), 0, 0);
  g.drawImageAt(assetManager->getScreens(), 0, 0);
}

void NamEditor::resized() {
  topBar.setBounds(0, 0, getWidth(), 40);
  debugLabel.setBounds(0, 40, getWidth(), 30);
}

void NamEditor::timerCallback() {
  debugLabel.setText(
      "Model Loaded: " +
          juce::String(audioProcessor.isNamModelLoaded() ? "True" : "False"),
      juce::dontSendNotification);

  repaint();
}

void NamEditor::sliderValueChanged(juce::Slider *slider) {}

void NamEditor::setMeterPosition(bool isOnMainScreen) {
  if (isOnMainScreen) {
    meterlnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour,
                       juce::Colours::ivory);
    meterlnf.setColour(foleys::LevelMeter::lmMeterOutlineColour,
                       juce::Colours::transparentWhite);
    meterlnf.setColour(foleys::LevelMeter::lmMeterBackgroundColour,
                       juce::Colours::transparentWhite);
    meterIn.setLookAndFeel(&meterlnf);
    meterOut.setLookAndFeel(&meterlnf);

    int meterHeight = 172;
    int meterWidth = 18;
    meterIn.setBounds(juce::Rectangle<int>(26, 174, meterWidth, meterHeight));
    meterOut.setBounds(juce::Rectangle<int>(getWidth() - meterWidth - 21, 174,
                                            meterWidth, meterHeight));
  } else {
    meterlnf2.setColour(foleys::LevelMeter::lmMeterGradientLowColour,
                        juce::Colours::ivory);
    meterIn.setLookAndFeel(&meterlnf2);
    meterOut.setLookAndFeel(&meterlnf2);

    int meterHeight = 255;
    int meterWidth = 20;
    meterIn.setBounds(20, (getHeight() / 2) - (meterHeight / 2) + 10,
                      meterWidth, meterHeight);
    meterOut.setBounds(getWidth() - 30,
                       (getHeight() / 2) - (meterHeight / 2) + 10, meterWidth,
                       meterHeight);
  }
}

void NamEditor::updateAfterPresetLoad() {}