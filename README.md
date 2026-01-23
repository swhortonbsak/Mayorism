<div id="namjuce-icon" align="center">
    <br />
    <img src="./Assets/ICON.png" alt="Mayerism icon" width="128"/>
    <h1>MAYERISM</h1>
    <h3>Open-source Mayer–inspired guitar plugin</h3>
</div>

<div id="badges" align="center">

[![current release](https://img.shields.io/github/release/timpelser/mayerism.svg)](https://github.com/timpelser/mayerism/releases)
[![license](https://img.shields.io/github/license/timpelser/mayerism.svg)](https://github.com/timpelser/mayerism/blob/master/LICENSE.txt)

</div>

<div id="description" align="center">

**MAYERISM is an open-source plugin inspired by John Mayer’s signature tones.**
Built on top of the [NeuralAmpModelerPlugin](https://github.com/sdatkinson/NeuralAmpModelerPlugin), it focuses on faithfully recreating the feel and response of Mayer’s most iconic amplifiers, paired with a carefully curated pedalboard for expressive, touch-sensitive playing.

</div>

## Overview

MAYERISM is designed as a single, cohesive guitar plugin. The goal is not endless options, but a ready-to-go platform that allows players to quickly achieve the inspiring tones of the last true guitar hero, without sacrificing feel, touch, or musical dynamics.

## Features

### Amp Section

* **John Mayer–inspired amp simulation**
  Neural amp modeling based on the tonal characteristics of Mayer’s most famous amplifiers, focusing on:

  * Clean to edge-of-breakup response
  * Touch sensitivity and pick dynamics
  * Smooth midrange and glassy high end

### Integrated Pedal Suite

* **Tube Screamer–style Overdrive**
  Classic mid-forward drive with Drive, Tone, and Level controls

* **Compressor**
  Transparent compression for sustain and control, ideal for clean and blues tones

* **Clean Boost**
  Transparent gain stage for pushing the amp or lifting solos

* **Chorus**
  Subtle modulation for width and movement, suited for clean and lightly driven tones

* **Delay**
  Musical delay for rhythmic repeats and ambient textures

* **Reverb**
  Smooth reverb for space and depth, from subtle room ambience to lush tails

### Tone Shaping & Utilities

* **Tone Stack EQ**
  Bass, Middle, and Treble controls tuned for Mayer-style voicing

* **Noise Gate**
  Controls noise and feedback without affecting playing dynamics

* **Doubler**
  Adds stereo width and depth

* **Input / Output Gain**
  Independent level control for gain staging and plugin integration

### Presets & Interface

* **Preset Management**
  Save and load presets that store your settings

* **Real-time Metering**
  Input and output level visualization

* **Multi-page Interface**
  Cleanly organized sections for:

  * Pre-effects
  * Amp
  * Post-effects

## License

This project is open source and distributed under the terms specified in the LICENSE file.

## Acknowledgements & Thanks

MAYERISM would not be possible without the incredible open-source work of others in the audio DSP and music technology community. Huge thanks to the authors and contributors of the following projects:

- [**NeuralAmpModelerPlugin**](https://github.com/sdatkinson/NeuralAmpModelerPlugin) 
  Created by Steven Atkinson and contributors  
  Used as the foundation for neural amp modeling and tone reproduction.

- [**KlonCentaur**](https://github.com/jatinchowdhury18/KlonCentaur)  
  Created by Jatin Chowdhury  
  Inspiration and reference for transparent overdrive design.

- [**TS-808-Ultra**](https://github.com/jstubbs/ts-808-ultra)  
  Created by James Stubbs  
  Reference implementation for classic Tube Screamer–style overdrive behavior.

- [**JUCE Framework**](https://github.com/juce/juce)  
  Developed by Raw Material Software  
  Used for cross-platform audio plugin development and user interface implementation.

- [**Two Rock Studio Signature + OX Box 2×12 Two Rock Cab (V2)**](https://www.tone3000.com/tones/two-rock-studio-signature-ox-box-2x12-two-rock-cab-v2-5367)  
  Originally shared on TONE3000 by danielnguyen
  Used as main amp emulation, slightly modified for use in MAYERISM.  

Each of these projects is licensed under its own open-source license.  
Please refer to their respective repositories and license files for full details.
