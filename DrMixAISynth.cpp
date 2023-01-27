#include "DrMixAISynth.h"
#include "IPlug/IPlug_include_in_plug_src.h"

#include <string.h>

DrMixAISynth::DrMixAISynth(void *instance):
  IPLUG_CTOR(kNumParams, 0, instance),
  m_sine(new SineSynth()),
  m_note_on(-1)
{
  AddParam(kParamFrequency, new IDoubleParam("Freq", 440, 20, 20000, 0, "Hz"));
}

void DrMixAISynth::SetSampleRate(double rate)
{
  IPlug::SetSampleRate(rate);
  m_sine->SetSampleRate(rate);
}

void DrMixAISynth::OnParamChange(int index)
{
  switch (index)
  {
    case kParamFrequency:
    {
      double freq = GetParam<IDoubleParam>(index)->Value();
      // SetFrequency(freq);
      break;
    }
  }
}

void DrMixAISynth::ProcessMidiMsg(const IMidiMsg *msg)
{
  switch (msg->mStatus >> 4)
  {
    case IMidiMsg::kNoteOn:
    if (msg->mData2)
    {
      int note = msg->mData1;

      double freq = pow(2, (double)(note - 69) / 12) * 440;
      SetFrequency(freq);

      m_note_on = note;
      break;
    }

    case IMidiMsg::kNoteOff:
    {
      int note = msg->mData1;

      if (note == m_note_on) m_note_on = -1;
      break;
    }
  }
}

void DrMixAISynth::ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples)
{
  if (m_note_on >= 0)
    Process(outputs[0], samples);
  else
    memset(outputs[0], 0, samples * sizeof(double));
}
