/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "eq_plagin/PluginEditor.h"
// #include <BinaryData.h>
#include "eq_plagin/PluginProcessor.h"
//==============================================================================

void LookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                                   float sliderPosProportional, float rotaryStartAngle,
                                   float rotaryEndAngle, juce::Slider &slider) {
  using namespace juce;

  auto bounds = Rectangle<float>(x, y, width, height);

  g.setColour(Colour(97u, 18u, 167u));
  g.fillEllipse(bounds);

  g.setColour(Colour(255u, 154u, 1u));
  g.drawEllipse(bounds, 1.f);

  if (auto *rswl = dynamic_cast<RotarySliderWithLabels *>(&slider)) {
    auto center = bounds.getCentre();

    Path p;

    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

    p.addRoundedRectangle(r, 2.f);
    jassert(rotaryStartAngle < rotaryEndAngle);

    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

    g.fillPath(p);

    g.setFont(rswl->getTextHeight());
    auto text = rswl->getDisplayString();
    auto strWidth = g.getCurrentFont().getStringWidth(text);

    r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
    r.setCentre(bounds.getCentre());

    g.setColour(Colours::black);
    g.fillRect(r);

    g.setColour(Colours::white);
    g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
  }
}

void RotarySliderWithLabels::paint(juce::Graphics &g) {
  using namespace juce;

  auto startAng = degreesToRadians(180.f + 45.f);
  auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

  auto range = getRange();

  auto sliderBounds = getSliderBounds();

  // g.setColour(Colours::red);
  // g.drawRect(getLocalBounds());
  // g.setColour(Colours::yellow);
  // g.drawRect(sliderBounds);

  getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(),
                                    sliderBounds.getWidth(), sliderBounds.getHeight(),
                                    jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                    startAng, endAng, *this);

  auto center = sliderBounds.toFloat().getCentre();
  auto radius = sliderBounds.getWidth() * 0.5f;

  g.setColour(Colour(0u, 172u, 1u));
  g.setFont(getTextHeight());

  auto numChoices = labels.size();
  for (int i = 0; i < numChoices; ++i) {
    auto pos = labels[i].pos;
    jassert(0.f <= pos);
    jassert(pos <= 1.f);
    auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

    auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f, ang);

    Rectangle<float> r;
    auto str = labels[i].label;
    r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
    r.setCentre(c);
    r.setY(r.getY() + getTextHeight());

    g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
  }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
  auto bounds = getLocalBounds();

  auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

  size -= getTextHeight() * 2;

  juce::Rectangle<int> r;

  r.setSize(size, size);
  r.setCentre(bounds.getCentreX(), 0);
  r.setY(2);

  return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const {
  if (auto *choiceParam = dynamic_cast<juce::AudioParameterChoice *>(param))
    return choiceParam->getCurrentChoiceName();

  juce::String str;

  bool addK = false;

  if (auto *floatParam = dynamic_cast<juce::AudioParameterFloat *>(param)) {
    float value = getValue();

    if (value > 999.f) {
      value /= 1000.f;
      addK = true;
    }

    str = juce::String(value, (addK ? 2 : 0));

  }

  else {
    jassertfalse;
  }

  if (suffix.isNotEmpty()) {
    str << " ";
    if (addK) str << "k";

    str << suffix;
  }
  return str;
}

//==============================================================================

ResponseCurveComponent::ResponseCurveComponent(TestpluginAudioProcessor &p)
    : audioProcessor(p),
      leftPathProducer(&audioProcessor.leftChannelFifo),
      rightPathProducer(&audioProcessor.rightChannelFifo) {
  const auto &params = audioProcessor.getParameters();
  for (auto param : params) {
    param->addListener(this);
  }

  updateChain();

  startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent() {
  const auto &params = audioProcessor.getParameters();
  for (auto param : params) {
    param->removeListener(this);
  }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue) {
  parametersChanged.set(true);
}
void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate) {
  juce::AudioBuffer<float> tempIncomingBuffer;

  while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0) {
    if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer)) {
      auto size = tempIncomingBuffer.getNumSamples();

      juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                        monoBuffer.getReadPointer(0, size),
                                        monoBuffer.getNumSamples() - size);

      juce::FloatVectorOperations::copy(
          monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
          tempIncomingBuffer.getReadPointer(0, 0), size);

      leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
    }
  }

  const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();

  const auto binWidth = sampleRate / (float)fftSize;

  while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0) {
    std::vector<float> fftData;
    if (leftChannelFFTDataGenerator.getFFTData(fftData)) {
      pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
    }
  }

  while (pathProducer.getNumPathsAvailable()) {
    pathProducer.getPath(leftChannelFFTPath);
  }
}
void ResponseCurveComponent::timerCallback() {
  auto fftBounds = getAnalysisArea().toFloat();
  auto sampleRate = audioProcessor.getSampleRate();

  leftPathProducer.process(fftBounds, sampleRate);
  rightPathProducer.process(fftBounds, sampleRate);

  if (parametersChanged.compareAndSetBool(false, true)) {
    // DBG("parameters changed");
    //  update the monoichaon

    updateChain();

    // signal a repeint
    // repaint();
  }

  repaint();
}

void ResponseCurveComponent::updateChain() {
  auto chainSettings = getChainSettings(audioProcessor.apvts);
  auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
  updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

  auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
  auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());

  updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients,
                  chainSettings.lowCutSlope);
  updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients,
                  chainSettings.highCutSlope);
}

void ResponseCurveComponent::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)

  // svgimg->drawWithin(g, getLocalBounds().toFloat(), juce::Justification::centred,
  // 1);

  using namespace juce;

  g.fillAll(Colours::black);

  g.drawImage(background, getLocalBounds().toFloat());

  auto responseArea = getAnalysisArea();

  auto w = responseArea.getWidth();

  auto &lowCut = monoChain.get<ChainPositions::LowCut>();
  auto &peak = monoChain.get<ChainPositions::Peak>();
  auto &highCut = monoChain.get<ChainPositions::HighCut>();

  auto sampleRate = audioProcessor.getSampleRate();

  std::vector<double> mags;

  mags.resize(w);
  for (int i = 0; i < w; ++i) {
    double mag = 1.f;
    auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

    if (!monoChain.isBypassed<ChainPositions::Peak>())
      mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!lowCut.isBypassed<0>())
      mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!lowCut.isBypassed<1>())
      mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!lowCut.isBypassed<2>())
      mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!lowCut.isBypassed<3>())
      mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!highCut.isBypassed<0>())
      mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!highCut.isBypassed<1>())
      mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!highCut.isBypassed<2>())
      mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    if (!highCut.isBypassed<3>())
      mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

    mags[i] = Decibels::gainToDecibels(mag);
  }

  Path responseCurve;

  const double outputMin = responseArea.getBottom();
  const double outputMax = responseArea.getY();

  auto map = [outputMin, outputMax](double input) {
    return jmap(input, -24.0, 24.0, outputMin, outputMax);
  };

  responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

  for (size_t i = 1; i < mags.size(); ++i) {
    responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
  }

  // Paint the left FFT chanel
  auto leftChannelFFTPath = leftPathProducer.getPath();
  leftChannelFFTPath.applyTransform(
      AffineTransform().translation(responseArea.getX(), responseArea.getY()));
  g.setColour(Colours::skyblue);
  g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

  // Paint the right  FFT chanel
  auto rightChannelFFTPath = rightPathProducer.getPath();
  rightChannelFFTPath.applyTransform(
      AffineTransform().translation(responseArea.getX(), responseArea.getY()));
  g.setColour(Colours::lightyellow);
  g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));

  g.setColour(Colours::orange);
  g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);

  g.setColour(Colours::white);

  g.strokePath(responseCurve, PathStrokeType(2.f));
}

void ResponseCurveComponent::resized() {
  using namespace juce;
  background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
  Graphics g(background);

  Array<float> freqs{20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};

  //-----------CACHE_GRID----------
  auto renderArea = getAnalysisArea();
  auto left = renderArea.getX();
  auto right = renderArea.getRight();
  auto top = renderArea.getY();
  auto bottom = renderArea.getBottom();
  auto width = renderArea.getWidth();

  Array<float> xs;
  for (auto f : freqs) {
    auto normX = mapFromLog10(f, 20.f, 20000.f);
    xs.add(left + width * normX);
  }
  //-----------CACHE_GRID----------

  g.setColour(Colours::dimgrey);

  for (auto x : xs) {
    g.drawVerticalLine(x, top, bottom);
  }

  Array<float> gain{-24, -12, 0, 12, 24};
  for (auto gDb : gain) {
    auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

    g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
    g.drawHorizontalLine(y, left, right);
  }

  g.setColour(Colours::lightgrey);
  const int fontHeight = 10;

  g.setFont(fontHeight);
  for (int i = 0; i < freqs.size(); ++i) {
    auto f = freqs[i];
    auto x = xs[i];

    bool addK = false;
    String str;
    if (f > 999.f) {
      f /= 1000.f;
      addK = true;
    }
    str << f;
    if (addK) str << "k";
    str << "Hz";

    auto textWidth = g.getCurrentFont().getStringWidth(str);

    Rectangle<int> r;
    r.setSize(textWidth, fontHeight);
    r.setCentre(x, 0);
    r.setY(1);

    g.drawFittedText(str, r, juce::Justification::centred, 1);
  }

  for (auto gDb : gain) {
    auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

    String str;
    if (gDb > 0) str << "+";
    str << gDb;

    auto textWidth = g.getCurrentFont().getStringWidth(str);

    Rectangle<int> r;
    r.setSize(textWidth, fontHeight);

    r.setX(getWidth() - textWidth);
    r.setCentre(r.getCentreX(), y);
    g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);

    g.drawFittedText(str, r, juce::Justification::centred, 1);

    str.clear();
    str << (gDb - 24.f);

    textWidth = g.getCurrentFont().getStringWidth(str);

    r.setSize(textWidth, fontHeight);
    r.setX(1);
    r.setCentre(r.getCentreX(), y);
    g.setColour(Colours::lightgrey);

    g.drawFittedText(str, r, juce::Justification::centred, 1);
  }
};

juce::Rectangle<int> ResponseCurveComponent::getRenderArea() {
  auto bounds = getLocalBounds();
  bounds.removeFromTop(12);
  bounds.removeFromBottom(2);
  bounds.removeFromLeft(20);
  bounds.removeFromRight(20);

  return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea() {
  auto bounds = getRenderArea();
  bounds.removeFromTop(4);
  bounds.removeFromBottom(4);
  return bounds;
}

//==============================================================================
TestpluginAudioProcessorEditor::TestpluginAudioProcessorEditor(TestpluginAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
      peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
      peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
      lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
      highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
      lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
      highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

      peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
      peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
      peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
      lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
      highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
      lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
      highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
      responseCurveComponent(audioProcessor)

{
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.

  peakFreqSlider.labels.add({0.f, "20Hz"});
  peakFreqSlider.labels.add({1.f, "20kHz"});

  peakGainSlider.labels.add({0.f, "-24dB"});
  peakGainSlider.labels.add({1.f, "+24dB"});

  peakQualitySlider.labels.add({0.f, "0.1"});
  peakQualitySlider.labels.add({1.f, "10.0"});

  lowCutFreqSlider.labels.add({0.f, "20Hz"});
  lowCutFreqSlider.labels.add({1.f, "20kHz"});

  highCutFreqSlider.labels.add({0.f, "20Hz"});
  highCutFreqSlider.labels.add({1.f, "20kHz"});

  lowCutSlopeSlider.labels.add({0.f, "12"});
  lowCutSlopeSlider.labels.add({1.f, "48"});

  highCutSlopeSlider.labels.add({0.f, "12"});
  highCutSlopeSlider.labels.add({1.f, "48"});

  for (auto comp : getComps()) {
    addAndMakeVisible(comp);
  }

  setSize(600, 500);
  // load Image from BinaryData
  // svgimg = juce::Drawable::createFromImageData(BinaryData::jucelogo_svg,
  //                                              BinaryData::jucelogo_svgSize);
}

TestpluginAudioProcessorEditor::~TestpluginAudioProcessorEditor() {}

//==============================================================================
void TestpluginAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)

  // svgimg->drawWithin(g, getLocalBounds().toFloat(), juce::Justification::centred,
  // 1);

  using namespace juce;

  g.fillAll(Colours::black);
}

void TestpluginAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..

  auto bounds = getLocalBounds();

  //  float hRatio = JUCE_LIVE_CONSTANT(33) / 100.f;  // JUCE_LIVE_CONSTANT
  float hRatio = 25.f / 100.f;

  auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

  responseCurveComponent.setBounds(responseArea);

  bounds.removeFromTop(5);

  auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
  auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

  lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
  lowCutSlopeSlider.setBounds(lowCutArea);
  highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
  highCutSlopeSlider.setBounds(highCutArea);

  peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
  peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
  peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component *> TestpluginAudioProcessorEditor::getComps() {
  return {&peakFreqSlider,    &peakGainSlider,    &peakQualitySlider,  &lowCutFreqSlider,
          &highCutFreqSlider, &lowCutSlopeSlider, &highCutSlopeSlider, &responseCurveComponent};
}
