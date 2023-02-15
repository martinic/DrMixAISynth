#include "DrMixAISynth.h"
#include "IPlug/IPlug_include_in_plug_src.h"

#include <string.h>

DrMixAISynth::DrMixAISynth(void *instance):
  IPLUG_CTOR(kNumParams, 0, instance),
  m_synth(new SawtoothSynth()),
  m_note_on(-1)
{
  AddParam(kParamCutoffFrequency, new IDoubleParam("Cutoff", 20000, 20, 20000, 0, "Hz"));
  AddParam(kParamResonance, new IDoubleParam("Resonance", 0.5, 0.5, 4.0, 1));
}

void DrMixAISynth::SetSampleRate(double rate)
{
  IPlug::SetSampleRate(rate);
  m_synth->SetSampleRate(rate);
}

void DrMixAISynth::SetBlockSize(int size)
{
  IPlug::SetBlockSize(size);
  m_midi_queue.Resize(GetBlockSize(), false);
}

void DrMixAISynth::OnParamChange(int index)
{
  switch (index)
  {
    case kParamCutoffFrequency:
    {
      double cutoff = GetParam<IDoubleParam>(index)->Value();
      SetCutoffFrequency(cutoff);
      break;
    }

    case kParamResonance:
    {
      double resonance = GetParam<IDoubleParam>(index)->Value();
      SetResonance(resonance);
      break;
    }
  }
}

void DrMixAISynth::Reset()
{
  m_synth->Reset();
}

void DrMixAISynth::ProcessMidiMsg(const IMidiMsg *msg)
{
  m_midi_queue.Add(msg);
}

void DrMixAISynth::ProcessMidiQueue(const IMidiMsg *msg)
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

    case IMidiMsg::kControlChange:
    {
      int cc = msg->mData1;

      if (cc == IMidiMsg::kAllNotesOff) m_note_on = -1;
      break;
    }
  }
}

void DrMixAISynth::ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples)
{
  for (int offset = 0; offset < samples;)
  {
    int next;

    for (;;)
    {
      if (m_midi_queue.Empty())
      {
        next = samples;
        break;
      }

      const IMidiMsg* msg = m_midi_queue.Peek();
      next = msg->mOffset;

      if (next > offset) break;

      ProcessMidiQueue(msg);
      m_midi_queue.Remove();
    }

    int block = next - offset;
    bool gate = m_note_on >= 0;
    Process(&outputs[0][offset], block, gate);

    offset = next;
  }

  memcpy(outputs[1], outputs[0], samples * sizeof(double));

  m_midi_queue.Flush(samples);
}
