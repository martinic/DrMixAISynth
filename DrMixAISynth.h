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
    output = applyAntiAliasing(output);
    m_phase += m_phaseIncrement;
    m_phase -= (int)m_phase;
    return output;
  }

private:
  float applyAntiAliasing(float sawtooth) {
    float polyBLEP;

    if (m_phase < m_phaseIncrement) {
      float x = m_phase / m_phaseIncrement - 1.0;
      polyBLEP = -(x*x);
    }
    else if (m_phase > 1.0 - m_phaseIncrement) {
      float x = (m_phase - 1.0) / m_phaseIncrement + 1.0;
      polyBLEP = x*x;
    }
    else {
      polyBLEP = 0.0;
    }

    return sawtooth - polyBLEP;
  }

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
    m_filter(1000, 1.0, sampleRate),

    m_sampleRate(sampleRate),
    m_noteOnTime(0.0)
  {
    m_attackTime = 0.1;
    m_decayTime = 0.2;
    m_sustainLevel = 0.5;
    m_releaseTime = 0.3;
  }

  void SetSampleRate(double rate)
  {
    m_sawtooth.setSampleRate(rate);
    m_filter.setSampleRate(rate);
  }

  void SetFrequency(double frequency) { m_sawtooth.setFrequency(frequency); }

  void SetAttackTime(double attack) { m_attackTime = attack; }
  void SetDecayTime(double decay) { m_decayTime = decay; }
  void SetSustainLevel(double sustain) { m_sustainLevel = sustain; }
  void SetReleaseTime(double release) { m_releaseTime = release; }

  void SetCutoffFrequency(double cutoff) { m_filter.setCutoffFrequency(cutoff); }
  void SetResonance(double resonance) { m_filter.setResonance(resonance); }

  void Reset()
  {
    m_sawtooth.reset();
    m_filter.reset();

    m_noteOnTime = 0.0;
  }

  void Attack() { m_noteOnTime = 0.0; }

  void Process(double *output, int samples, bool gate)
  {
    // The synthesizer's rendering loop
    float sample;
    for (int i = 0; i < samples; i++)
    {
      // Calculate the envelope value for each sample
      float time = i / m_sampleRate;
      float envelope = adsrEnvelope(time, m_noteOnTime);
      sample = m_sawtooth.getNextSample() * envelope;

      sample = gate ? sample : 0.0;
      output[i] = m_filter.process(sample);
    }

    m_noteOnTime -= samples / m_sampleRate;
  }

private:
  // A function to calculate the envelope value at a given time
  float adsrEnvelope(float time, float noteOnTime)
  {
    float deltaTime = time - m_noteOnTime;
    if (deltaTime < m_attackTime)
    {
      // Attack phase
      return deltaTime / m_attackTime;
    }
    else if (deltaTime < m_attackTime + m_decayTime)
    {
      // Decay phase
      return 1.0 - (1.0 - m_sustainLevel) * (deltaTime - m_attackTime) / m_decayTime;
    }
    else
    {
      // Sustain or Release phase
      return m_sustainLevel * exp(-(deltaTime - m_attackTime - m_decayTime) / m_releaseTime);
    }
  }

  SawtoothOscillator m_sawtooth;
  LowPassFilter m_filter;

  float m_sampleRate;
  float m_noteOnTime;

  // ADSR parameters
  float m_attackTime; // Time for the amplitude to reach its peak
  float m_decayTime; // Time for the amplitude to decay from peak to sustain level
  float m_sustainLevel; // Level at which the amplitude sustains
  float m_releaseTime; // Time for the amplitude to decay from sustain level to zero
};

enum EParams
{
  kParamAttackTime = 0,
  kParamDecayTime,
  kParamSustainLevel,
  kParamReleaseTime,

  kParamCutoffFrequency,
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

  void SetAttackTime(double attack) { m_synth->SetAttackTime(attack); }
  void SetDecayTime(double decay) { m_synth->SetDecayTime(decay); }
  void SetSustainLevel(double sustain) { m_synth->SetSustainLevel(sustain); }
  void SetReleaseTime(double release) { m_synth->SetReleaseTime(release); }

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
