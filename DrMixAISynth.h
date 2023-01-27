#include "WDL/wdltypes.h"
#include "WDL/ptrlist.h"

class SineSynth
{
public:
  SineSynth() : m_frequency(0), m_phase(0) {}

  void SetFrequency(double frequency) { m_frequency = frequency; }

  void Process(double *output, int samples)
  {
    double phase_step = m_frequency / GetSampleRate();

    for (int i = 0; i < samples; i++)
    {
      output[i] = sin(m_phase);
      m_phase += phase_step;
    }
  }

private:
  double m_frequency;
  double m_phase;
};

class MySynth : public WDL_Synth
{
public:
  MySynth() : m_sine(new SineSynth()) {}
  ~MySynth() { delete m_sine; }

  void SetFrequency(double frequency) { m_sine->SetFrequency(frequency); }

  void Process(double *output, int samples)
  {
    m_sine->Process(output, samples);
  }

private:
  SineSynth *m_sine;
};
