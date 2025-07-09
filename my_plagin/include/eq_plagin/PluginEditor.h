/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "eq_plagin/PluginProcessor.h"

enum FFTOrder {

  order2048 = 11,
  order4096 = 12,
  order8192 = 13,
};

//=============================================================================
template <typename BlockType>
struct FFTDataGenerator {
  void produceFFTDataForRendering(const juce::AudioBuffer<float> &audioData,
                                  const float negativeInfinity) {
    const auto fftSize = getFFTSize();

    fftData.assign(fftData.size(), 0);
    auto *readIndex = audioData.getReadPointer(0);
    std::copy(readIndex, readIndex + fftSize, fftData.begin());

    window->multiplyWithWindowingTable(fftData.data(), fftSize);

    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());

    int numBins = (int)fftSize / 2;

    for (int i = 0; i < numBins; ++i) {
      fftData[i] /= (float)numBins;
    }

    for (int i = 0; i < numBins; ++i) {
      fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
    }

    fftDataFifo.push(fftData);
  }

  void changeOrder(FFTOrder newOrder) {
    order = newOrder;
    auto fftSize = getFFTSize();

    forwardFFT = std::make_unique<juce::dsp::FFT>(order);
    window = std::make_unique<juce::dsp::WindowingFunction<float>>(
        fftSize, juce::dsp::WindowingFunction<float>::hann);

    fftData.clear();
    fftData.resize(fftSize * 2, 0);

    fftDataFifo.prepare(fftData.size());
  }

  int getFFTSize() const { return 1 << order; }
  int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }

  bool getFFTData(BlockType &fftData) { return fftDataFifo.pull(fftData); }

 private:
  FFTOrder order;
  BlockType fftData;
  std::unique_ptr<juce::dsp::FFT> forwardFFT;
  std::unique_ptr<juce::dsp::WindowingFunction<float>> window;

  Fifo<BlockType> fftDataFifo;
};
//=============================================================================

template <typename PathType>
struct AnalyzerPathGenerator {
  void generatePath(const std::vector<float> &renderData, juce::Rectangle<float> fftBounds,
                    int fftSize, float binWidth, float negativeInfinity) {
    auto top = fftBounds.getY();
    auto bottom = fftBounds.getHeight();
    auto left = fftBounds.getX();
    auto width = fftBounds.getWidth();

    int numBins = (int)fftSize / 2;

    PathType p;
    p.preallocateSpace(3 * (int)fftBounds.getWidth());

    auto map = [bottom, top, negativeInfinity](float v) {
      return juce::jmap(v, negativeInfinity, 0.f, float(bottom), top);
    };

    auto y = map(renderData[0]);

    jassert(!std::isnan(y) && !std::isinf(y));

    p.startNewSubPath(left, y);

    const int pathResolution = 2;

    for (int binNum = 1; binNum < numBins; binNum += pathResolution) {
      y = map(renderData[binNum]);

      jassert(!std::isnan(y) && !std::isinf(y));

      if (!std::isnan(y) && !std::isinf(y)) {
        auto binFreq = binNum * binWidth;
        auto normalisedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
        int binX = std::floor(normalisedBinX * width);
        p.lineTo(left + binX, y);
      }
    }
    pathFifo.push(p);
  }

  int getNumPathsAvailable() const { return pathFifo.getNumAvailableForReading(); }

  bool getPath(PathType &path) { return pathFifo.pull(path); }

 private:
  Fifo<PathType> pathFifo;
};

//=============================================================================
struct LookAndFeel : public juce::LookAndFeel_V4 {
  virtual void drawRotarySlider(juce::Graphics &, int x, int y, int width, int height,
                                float sliderPosProportional, float rotaryStartAngle,
                                float rotaryEndAngle, juce::Slider &) override;
};

struct RotarySliderWithLabels : juce::Slider {
  RotarySliderWithLabels(juce::RangedAudioParameter &rap, const juce::String &unitSuffix)
      : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix) {
    setLookAndFeel(&lnf);
  };

  ~RotarySliderWithLabels() override { setLookAndFeel(nullptr); }

  struct LabelPos {
    float pos;
    juce::String label;
  };

  juce::Array<LabelPos> labels;
  void paint(juce::Graphics &g) override;

  juce::Rectangle<int> getSliderBounds() const;
  int getTextHeight() const { return 14; }
  juce::String getDisplayString() const;

 private:
  LookAndFeel lnf;
  juce::RangedAudioParameter *param;
  juce::String suffix;
};

struct PathProducer {
  PathProducer(SingleChannelSampleFifo<TestpluginAudioProcessor::BlockType> *scsf)
      : leftChannelFifo(scsf) {
    leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
    monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
  void process(juce::Rectangle<float> fftBounds, double sampleRate);
  juce::Path getPath() { return leftChannelFFTPath; }

 private:
  SingleChannelSampleFifo<TestpluginAudioProcessor::BlockType> *leftChannelFifo;

  juce::AudioBuffer<float> monoBuffer;

  FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

  AnalyzerPathGenerator<juce::Path> pathProducer;

  juce::Path leftChannelFFTPath;
};

struct ResponseCurveComponent : juce::Component,
                                juce::AudioProcessorParameter::Listener,
                                juce::Timer {
  ResponseCurveComponent(TestpluginAudioProcessor &);
  ~ResponseCurveComponent() override;

  void parameterValueChanged(int parameterIndex, float newValue) override;

  void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};

  void timerCallback() override;

  void paint(juce::Graphics &g) override;
  void resized() override;

 private:
  TestpluginAudioProcessor &audioProcessor;
  juce::Atomic<bool> parametersChanged{false};

  MonoChain monoChain;
  void updateChain();

  juce::Image background;
  juce::Rectangle<int> getRenderArea();
  juce::Rectangle<int> getAnalysisArea();

  PathProducer leftPathProducer, rightPathProducer;
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
