/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "eq_plagin/PluginProcessor.h"

#include "eq_plagin/PluginEditor.h"

//==============================================================================
TestpluginAudioProcessor::TestpluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
}

TestpluginAudioProcessor::~TestpluginAudioProcessor() {}

//==============================================================================
const juce::String TestpluginAudioProcessor::getName() const { return JucePlugin_Name; }

bool TestpluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool TestpluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool TestpluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double TestpluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TestpluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int TestpluginAudioProcessor::getCurrentProgram() { return 0; }

void TestpluginAudioProcessor::setCurrentProgram(int index) {}

const juce::String TestpluginAudioProcessor::getProgramName(int index) { return {}; }

void TestpluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {}

//==============================================================================
void TestpluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..

  juce::dsp::ProcessSpec spec;
  using namespace Steinberg;

  spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
  spec.numChannels = 1;
  spec.sampleRate = sampleRate;
  leftChain.prepare(spec);
  rightChain.prepare(spec);

  auto chainSettings = getChainSettings(apvts);

  updateFilters();

  leftChannelFifo.prepare(samplesPerBlock);
  rightChannelFifo.prepare(samplesPerBlock);

  osc.initialise([](float x) { return std::sin(x); });

  spec.numChannels = getTotalNumOutputChannels();

  osc.prepare(spec);
  osc.setFrequency(100.f);
}

void TestpluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestpluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) return false;
#endif

  return true;
#endif
}
#endif

void TestpluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // This is the place where you'd normally do the guts of your plugin's
  // audio processing...
  // Make sure to reset the state if your inner loop is processing
  // the samples and the outer loop is handling the channels.
  // Alternatively, you can process the samples with the channels
  // interleaved by keeping the same state.

  updateFilters();

  juce::dsp::AudioBlock<float> block(buffer);

  
  // buffer.clear();
  // juce::dsp::ProcessContextReplacing<float> stereoContex(block);
  // osc.process(stereoContex); 



  auto leftBlock = block.getSingleChannelBlock(0);
  auto rightBlock = block.getSingleChannelBlock(1);

  juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
  juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

  leftChain.process(leftContext);
  rightChain.process(rightContext);

  leftChannelFifo.update(buffer);
  rightChannelFifo.update(buffer);

  for (int channel = 0; channel < totalNumInputChannels; ++channel) {
    auto *channelData = buffer.getWritePointer(channel);

    // ..do something to the data...
  }
}

//==============================================================================
bool TestpluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *TestpluginAudioProcessor::createEditor() {
  return new TestpluginAudioProcessorEditor(*this);
  // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TestpluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.

  juce::MemoryOutputStream mos(destData, true);
  apvts.state.writeToStream(mos);
}

void TestpluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.

  juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
  if (tree.isValid()) {
    apvts.replaceState(tree);
    updateFilters();
  }
}

//======================My_user_code_begin_here================================

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState &apvts) {
  ChainSettings settings;

  settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
  settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
  settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
  settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
  settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
  settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
  settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

  return settings;
}

Coefficients makePeakFilter(const ChainSettings &chainSettings, double sampleRate) {
  return juce::dsp::IIR::Coefficients<float>::makePeakFilter(
      sampleRate, chainSettings.peakFreq, chainSettings.peakQuality,
      juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
}

void TestpluginAudioProcessor::updatePeakFilter(const ChainSettings &chainSettings) {
  auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());

  updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
  updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void updateCoefficients(Coefficients &old, const Coefficients &replacements) {
  *old = *replacements;
}

void TestpluginAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings) {
  auto lowCutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());

  auto &leftLowCut = leftChain.get<ChainPositions::LowCut>();
  auto &rightLowCut = rightChain.get<ChainPositions::LowCut>();

  updateCutFilter(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
  updateCutFilter(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
}

void TestpluginAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings) {
  auto highCutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());

  auto &leftHighCut = leftChain.get<ChainPositions::HighCut>();
  auto &rightHighCut = rightChain.get<ChainPositions::HighCut>();
  updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
  updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
}

void TestpluginAudioProcessor::updateFilters() {
  auto chainSettings = getChainSettings(apvts);

  updatePeakFilter(chainSettings);
  updateLowCutFilters(chainSettings);
  updateHighCutFilters(chainSettings);
}

juce::AudioProcessorValueTreeState::ParameterLayout
TestpluginAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "LowCut Freq", "LowCut Freq", juce::NormalisableRange<float>(20.f, 20'000.f, 1.f, 0.25f),
      20.f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(20.f, 20'000.f, 1.f, 0.25f),
      20.000f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "Peak Freq", "Peak Freq", juce::NormalisableRange<float>(20.f, 20'000.f, 1.f, 0.25f), 750.f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "Peak Gain", "Peak Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "Peak Quality", "Peak Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

  juce::StringArray stringArray;
  for (int j = 0; j < 4; ++j) {
    juce::String str;
    str << (12 + 12 * j);
    str << " db/Oct";
    stringArray.add(str);
  }

  layout.add(
      std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
  layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope",
                                                          stringArray, 0));

  return layout;
}

//======================My_user_code_end_here================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new TestpluginAudioProcessor(); }
