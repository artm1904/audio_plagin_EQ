/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
 */
class TestpluginAudioProcessor : public juce::AudioProcessor {
 public:
  //==============================================================================
  TestpluginAudioProcessor();
  ~TestpluginAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  //======================My_user_code_begin_here================================

  static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Patametrs", createParameterLayout()};

  //======================My_user_code_end_here================================

 private:
  //======================My_user_code_begin_here================================

  using Filter = juce::dsp::IIR::Filter<float>;
  using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
  using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

  MonoChain leftChain, rightChain;

  //======================My_user_code_end_here================================

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestpluginAudioProcessor)
};
