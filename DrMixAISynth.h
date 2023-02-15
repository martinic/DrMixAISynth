#include "IPlug/IPlug_include_in_plug_hdr.h"
#include "IPlug/IMidiQueue.h"

#include <math.h>

#include "WDL/wdltypes.h"
#include "WDL/ptrlist.h"

class SawtoothOscillator {
public:
  SawtoothOscillator(float frequency, float sampleRate) : m_frequency(frequency), m_sampleRate(sampleRate) {
    m_phase = 0.0;
    m_phaseIncrement = frequency / sampleRate;
  }

  void reset() { m_phase = 0.0; }

  void setFrequency(float frequency) {
    m_frequency = frequency;
    m_phaseIncrement = frequency / m_sampleRate;
  }

  void setSampleRate(float sampleRate) {
    m_sampleRate = sampleRate;
    m_phaseIncrement = m_frequency / sampleRate;
  }

  float getNextSample() {
    float output = 2.0 * m_phase - 1.0; // Output a sawtooth wave between -1 and 1
    m_phase += m_phaseIncrement;
    m_phase -= (int)m_phase;
    return output;
  }

private:
  float m_frequency;
  float m_sampleRate;
  float m_phase;
  float m_phaseIncrement;
};

class LowPassFilter {
public:
  LowPassFilter(float cutoffFrequency, float resonance, float sampleRate) :
    m_cutoffFrequency(cutoffFrequency),
    m_resonance(resonance),
    m_sampleRate(sampleRate),
    m_cutoffFrequencyTarget(cutoffFrequency),
    m_resonanceTarget(resonance)
  {
    reset();
    calculateSmoothingFactor();
    calculateCoefficients();
  }

  void setCutoffFrequency(float cutoffFrequency) { m_cutoffFrequencyTarget = cutoffFrequency; }
  void setResonance(float resonance) { m_resonanceTarget = resonance; }

  void setSampleRate(float sampleRate) {
    m_sampleRate = sampleRate;

    reset();
    calculateSmoothingFactor();
    calculateCoefficients();
  }

  float process(float input) {
    // Smooth cutoff frequency/resonance changes
    if (m_cutoffFrequency != m_cutoffFrequencyTarget ||
        m_resonance != m_resonanceTarget)
    {
      m_cutoffFrequency = applySmoothing(m_cutoffFrequency, m_cutoffFrequencyTarget);
      m_resonance = applySmoothing(m_resonance, m_resonanceTarget);

      calculateCoefficients();
    }

    // Calculate output using Direct Form I structure
    float output = m_b0 * input + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;
    
    // Update state variables
    m_x2 = m_x1;
    m_x1 = input;
    m_y2 = m_y1;
    m_y1 = output;

    return output;
  }

  void reset() {
    // Reset state variables to 0
    m_x1 = m_x2 = m_y1 = m_y2 = 0.0;
  }

private:
  float applySmoothing(float currentValue, float targetValue) {
    return (targetValue - currentValue) * m_smoothingFactor + currentValue;
  }

  void calculateSmoothingFactor() {
    m_smoothingFactor = 1.0 - exp(-5.0 / (0.100 /* 100 ms */ * m_sampleRate));
  }

  void calculateCoefficients() {
    // Calculate filter coefficients based on cutoff frequency and resonance
    float omega = 2.0 * M_PI * m_cutoffFrequency / m_sampleRate;
    omega = omega < 0.0 ? 0.0 : omega;
    omega = omega > 0.98 * M_PI ? 0.98 * M_PI : omega;
    float alpha = sin(omega) / (2.0 * m_resonance);
    float cosw = cos(omega);
    float a0inv = 1.0 / (1.0 + alpha);
    m_b0 = (1.0 - cosw) / 2.0 * a0inv;
    m_b1 = (1.0 - cosw) * a0inv;
    m_b2 = (1.0 - cosw) / 2.0 * a0inv;
    m_a1 = -2.0 * cosw * a0inv;
    m_a2 = (1.0 - alpha) * a0inv;
  }

  float m_cutoffFrequency;
  float m_resonance;
  float m_sampleRate;

  // Cutoff frequency/resonance smoothing
  float m_cutoffFrequencyTarget;
  float m_resonanceTarget;
  float m_smoothingFactor;

  float m_x1, m_x2, m_y1, m_y2; // State variables
  float m_b0, m_b1, m_b2, m_a1, m_a2; // Filter coefficients
};

class SawtoothSynth
{
public:
  SawtoothSynth(double sampleRate = 44100) :
    m_sawtooth(440, sampleRate),
    m_filter(1000, 1.0, sampleRate)
  {}

  void SetSampleRate(double rate)
  {
    m_sawtooth.setSampleRate(rate);
    m_filter.setSampleRate(rate);
  }

  void SetFrequency(double frequency) { m_sawtooth.setFrequency(frequency); }
  void SetCutoffFrequency(double cutoff) { m_filter.setCutoffFrequency(cutoff); }
  void SetResonance(double resonance) { m_filter.setResonance(resonance); }

  void Reset()
  {
    m_sawtooth.reset();
    m_filter.reset();
  }

  void Process(double *output, int samples, bool gate)
  {
    float sample;
    for (int i = 0; i < samples; i++)
    {
      sample = m_sawtooth.getNextSample();
      sample = gate ? sample : 0.0;
      output[i] = m_filter.process(sample);
    }
  }

private:
  SawtoothOscillator m_sawtooth;
  LowPassFilter m_filter;
};

enum EParams
{
  kParamCutoffFrequency = 0,
  kParamResonance,

  kNumParams
};

class DrMixAISynth : public IPlug
{
public:
  DrMixAISynth(void *instance);
  ~DrMixAISynth() { delete m_synth; }

  void SetSampleRate(double rate);
  void SetBlockSize(int size);

  void OnParamChange(int index);

  void SetFrequency(double frequency) { m_synth->SetFrequency(frequency); }
  void SetCutoffFrequency(double cutoff) { m_synth->SetCutoffFrequency(cutoff); }
  void SetResonance(double resonance) { m_synth->SetResonance(resonance); }

  void Reset();

  void ProcessMidiMsg(const IMidiMsg *msg);
  void ProcessMidiQueue(const IMidiMsg *msg);

  void ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples);

  void Process(double *output, int samples, bool gate)
  {
    m_synth->Process(output, samples, gate);
  }

private:
  SawtoothSynth *m_synth;

  IMidiQueue m_midi_queue;
  int m_note_on;
};
