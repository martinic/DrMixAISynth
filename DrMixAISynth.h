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
  LowPassFilter(float cutoffFrequency, float resonance) : m_cutoffFrequency(cutoffFrequency), m_resonance(resonance) {
    reset();
  }

  float process(float input) {
    // Calculate output using Direct Form II structure
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

    // Calculate filter coefficients based on cutoff frequency and resonance
    float omega = 2.0 * M_PI * m_cutoffFrequency;
    float alpha = sin(omega) / (2.0 * m_resonance);
    float cosw = cos(omega);
    float a0inv = 1.0 / (1.0 + alpha);
    m_b0 = (1.0 - cosw) / 2.0 * a0inv;
    m_b1 = (1.0 - cosw) * a0inv;
    m_b2 = (1.0 - cosw) / 2.0 * a0inv;
    m_a1 = -2.0 * cosw * a0inv;
    m_a2 = (1.0 - alpha) * a0inv;
  }

private:
  float m_cutoffFrequency;
  float m_resonance;
  float m_x1, m_x2, m_y1, m_y2; // State variables
  float m_b0, m_b1, m_b2, m_a1, m_a2; // Filter coefficients
};

class SawtoothSynth
{
public:
  SawtoothSynth() : m_sawtooth(440, 44100) {}

  void SetSampleRate(double rate) { m_sawtooth.setSampleRate(rate); }
  void SetFrequency(double frequency) { m_sawtooth.setFrequency(frequency); }

  void Reset() { m_sawtooth.reset(); }

  void Process(double *output, int samples)
  {
    for (int i = 0; i < samples; i++)
    {
      output[i] = m_sawtooth.getNextSample();
    }
  }

private:
  SawtoothOscillator m_sawtooth;
};

enum EParams
{
  kParamFrequency = 0,
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

  void Reset();

  void ProcessMidiMsg(const IMidiMsg *msg);
  void ProcessMidiQueue(const IMidiMsg *msg);

  void ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples);

  void Process(double *output, int samples)
  {
    m_synth->Process(output, samples);
  }

private:
  SawtoothSynth *m_synth;

  IMidiQueue m_midi_queue;
  int m_note_on;
};
