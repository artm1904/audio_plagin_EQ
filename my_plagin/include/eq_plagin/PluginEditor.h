/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "eq_plagin/PluginProcessor.h"

struct LookAndFeel : public juce::LookAndFeel_V4 {
  virtual void drawRotarySlider(juce::Graphics &, int x, int y, int width, int height,
                                float sliderPosProportional, float rotaryStartAngle,
                                float rotaryEndAngle, juce::Slider &) override ;
};

struct RotarySliderWithLabels : juce::Slider {
  RotarySliderWithLabels(juce::RangedAudioParameter &rap, const juce::String &unitSuffix)
      : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix) {
    setLookAndFeel(&lnf);
  };

  ~RotarySliderWithLabels() { setLookAndFeel(nullptr); }
  

  struct LabelPos {
    float pos;
    juce::String label;
  };

  juce::Array<LabelPos> labels;
  void paint(juce::Graphics &g) override ;

  juce::Rectangle<int> getSliderBounds() const;
  int getTextHeight() const { return 14; }
  juce::String getDisplayString() const;

  

 private:
  LookAndFeel lnf;
  juce::RangedAudioParameter *param;
  juce::String suffix;
};

struct ResponseCurveComponent : juce::Component,
                                juce::AudioProcessorParameter::Listener,
                                juce::Timer {
  ResponseCurveComponent(TestpluginAudioProcessor &);
  ~ResponseCurveComponent();

  void parameterValueChanged(int parameterIndex, float newValue) override;

  void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};

  void timerCallback() override;

  void paint(juce::Graphics &g) override;

 private:
  TestpluginAudioProcessor &audioProcessor;
  juce::Atomic<bool> parametersChanged{false};

  MonoChain monoChain;
  void updateChain();
};

//==============================================================================
/**
 */
class TestpluginAudioProcessorEditor : public juce::AudioProcessorEditor {
 public:
  TestpluginAudioProcessorEditor(TestpluginAudioProcessor &);
  ~TestpluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  TestpluginAudioProcessor &audioProcessor;

  RotarySliderWithLabels peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider,
      highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;

  using APVTS = juce::AudioProcessorValueTreeState;
  using Attachment = APVTS::SliderAttachment;
  Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment,
      lowCutFreqSliderAttachment, highCutFreqSliderAttachment, lowCutSlopeSliderAttachment,
      highCutSlopeSliderAttachment;

  ResponseCurveComponent responseCurveComponent;

  std::vector<juce::Component *> getComps();

  // std::unique_ptr<juce::Drawable> svgimg;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestpluginAudioProcessorEditor)
};
