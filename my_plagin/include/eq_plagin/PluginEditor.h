/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "eq_plagin/PluginProcessor.h"

struct CustomRotarySlider : juce::Slider {
  CustomRotarySlider()
      : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox) {}
};

//==============================================================================
/**
 */
class TestpluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       juce::AudioProcessorParameter::Listener,
                                       juce::Timer {
 public:
  TestpluginAudioProcessorEditor(TestpluginAudioProcessor &);
  ~TestpluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

  void parameterValueChanged(int parameterIndex, float newValue) override;

  void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};

  void timerCallback() override;

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  TestpluginAudioProcessor &audioProcessor;

  juce::Atomic<bool> parametersChanged{false};

  CustomRotarySlider peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider,
      highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;

  using APVTS = juce::AudioProcessorValueTreeState;
  using Attachment = APVTS::SliderAttachment;
  Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment,
      lowCutFreqSliderAttachment, highCutFreqSliderAttachment, lowCutSlopeSliderAttachment,
      highCutSlopeSliderAttachment;

  std::vector<juce::Component *> getComps();

  MonoChain monoChain;

  // std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestpluginAudioProcessorEditor)
};
