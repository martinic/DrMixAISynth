#include "IPlug/IPlug_include_in_plug_hdr.h"
#include "IPlug/IMidiQueue.h"

#include <math.h>

#include "WDL/wdltypes.h"
#include "WDL/ptrlist.h"

class SineSynth
{
public:
  SineSynth() : m_sample_rate(44100), m_frequency(0), m_phase(0) {}

  void SetSampleRate(double rate) { m_sample_rate = rate; }
  double GetSampleRate() { return m_sample_rate; }

  void SetFrequency(double frequency) { m_frequency = frequency; }

  void Process(double *output, int samples)
  {
    double phase_step = m_frequency / GetSampleRate();

    for (int i = 0; i < samples; i++)
    {
      output[i] = sin(2*M_PI * m_phase);
      m_phase += phase_step;
    }
  }

private:
  double m_sample_rate;
  double m_frequency;
  double m_phase;
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
  ~DrMixAISynth() { delete m_sine; }

  void SetSampleRate(double rate);
  void SetBlockSize(int size);

  void OnParamChange(int index);
  void SetFrequency(double frequency) { m_sine->SetFrequency(frequency); }

  void ProcessMidiMsg(const IMidiMsg *msg);
  void ProcessMidiQueue(const IMidiMsg *msg);

  void ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples);

  void Process(double *output, int samples)
  {
    m_sine->Process(output, samples);
  }

private:
  SineSynth *m_sine;

  IMidiQueue m_midi_queue;
  int m_note_on;
};
