#include "DrMixAISynth.h"
#include "IPlug/IPlug_include_in_plug_src.h"

DrMixAISynth::DrMixAISynth(void *instance):
  IPLUG_CTOR(kNumParams, 0, instance),
  m_sine(new SineSynth())
{
  AddParam(kParamFrequency, new IDoubleParam("Freq", 440, 20, 20000, 0, "Hz"));
}

void DrMixAISynth::OnParamChange(int index)
{
  switch (index)
  {
    case kParamFrequency:
    {
      double freq = GetParam<IDoubleParam>(index)->Value();
      SetFrequency(freq);
      break;
    }
  }
}

void DrMixAISynth::ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples)
{
  Process(outputs[0], samples);
}
