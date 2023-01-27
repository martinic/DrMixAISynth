#include "DrMixAISynth.h"
#include "IPlug/IPlug_include_in_plug_src.h"

DrMixAISynth::DrMixAISynth(void *instance):
  IPLUG_CTOR(0, 0, instance),
  m_sine(new SineSynth())
{
}

void DrMixAISynth::ProcessDoubleReplacing(const double *const *inputs, double *const *outputs, int samples)
{
  Process(outputs[0], samples);
}
