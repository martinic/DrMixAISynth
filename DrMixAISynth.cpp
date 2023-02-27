#include "DrMixAISynth.h"
#include "IPlug/IPlug_include_in_plug_src.h"

#include "WDL/denormal.h"
#include <string.h>

class IKnobCustomControl: public IKnobMultiControl
{
public:
	IKnobCustomControl(IPlugBase *pPlug, int x, int y, int paramIdx = -1, const IBitmap *pBitmap = NULL)
	: IKnobMultiControl(pPlug, x, y, paramIdx, pBitmap) {}

	void PromptUserInput()
	{
		mPlug->GetGUI()->PromptUserInput(this, mPlug->GetParam(mParamIdx), &mRECT, IGraphics::kPromptCustomWidth);
		Redraw();
	}
};

DrMixAISynth::DrMixAISynth(void *instance):
  IPLUG_CTOR(kNumParams, 1, instance),
  m_synth(new SawtoothSynth()),
  m_note_on(-1)
{
  // Plugin parameters

  IBoolParam *pBypassParam = new IBoolParam("Bypass", false);
  pBypassParam->SetGlobal(true);
  AddParam(kParamBypass, pBypassParam);

  AddParam(kParamEnvelope, new IBoolParam("Envelope", false));
  AddParam(kParamAttackTime, new IDoubleExpParam(3, "Attack", 100, 1, 5000, 0, "ms"));
  AddParam(kParamDecayTime, new IDoubleExpParam(3, "Decay", 200, 1, 5000, 0, "ms"));
  AddParam(kParamSustainLevel, new IDoubleParam("Sustain", -6.0, -72.0, 0.0, 1, "dB"));
  AddParam(kParamReleaseTime, new IDoubleExpParam(3, "Release", 300, 1, 5000, 0, "ms"));

  AddParam(kParamCutoffFrequency, new IDoubleExpParam(6, "Cutoff", 20000, 20, 20000, 0, "Hz"));
  AddParam(kParamResonance, new IDoubleParam("Resonance", 0.5, 0.5, 4.0, 1));

  AddParam(kParamLFOFrequency, new IDoubleExpParam(3, "LFO Rate", 2, 0.1, 10, 2, "Hz"));
  AddParam(kParamLFOAmplitude, new IDoubleParam("LFO Depth", 0, 0, 1000, 0, "Hz"));

  MakeDefaultPreset("Default");

  // GUI

  IGraphics *pGraphics = MakeGraphics(this, 1200, 680);

  pGraphics->SetDefaultScale(IGraphics::kScaleHalf);
  pGraphics->EnableTooltips(true);
  pGraphics->HandleMouseWheel(IGraphics::kMouseWheelModKey);

  IBitmap background(BACKGROUND_PNG_ID, pGraphics->Width(), pGraphics->Height());
  pGraphics->AttachBackground(new IBackgroundControl(this, &background));

  // Envelope switch/knobs

  IBitmap switchBitmap(SWITCH_PNG_ID, 90, 32);
  IControl *pSwitchControl = new ISwitchControl(this, 62, 72, kParamEnvelope, &switchBitmap);
  pSwitchControl->SetTooltip("Envelope On/Off");
  pGraphics->AttachControl(pSwitchControl);

  IBitmap knobBitmap(KNOB_PNG_ID, 88, 88, 129);
  IControl *pKnobControl;

  pKnobControl = new IKnobCustomControl(this, 62, 150, kParamAttackTime, &knobBitmap);
  pKnobControl->SetTooltip("Attack Time");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 62, 280, kParamDecayTime, &knobBitmap);
  pKnobControl->SetTooltip("Decay Time");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 62, 410, kParamSustainLevel, &knobBitmap);
  pKnobControl->SetTooltip("Sustain Level");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 62, 540, kParamReleaseTime, &knobBitmap);
  pKnobControl->SetTooltip("Release Time");
  pGraphics->AttachControl(pKnobControl);

  // Filter knobs

  pKnobControl = new IKnobCustomControl(this, 214, 108, kParamCutoffFrequency, &knobBitmap);
  pKnobControl->SetTooltip("Cutoff Frequency");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 214, 238, kParamResonance, &knobBitmap);
  pKnobControl->SetTooltip("Resonance");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 214, 368, kParamLFOFrequency, &knobBitmap);
  pKnobControl->SetTooltip("LFO Rate");
  pGraphics->AttachControl(pKnobControl);

  pKnobControl = new IKnobCustomControl(this, 214, 498, kParamLFOAmplitude, &knobBitmap);
  pKnobControl->SetTooltip("LFO Depth");
  pGraphics->AttachControl(pKnobControl);

  AttachGraphics(pGraphics);
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
    case kParamEnvelope:
    {
      bool enable = GetParam<IBoolParam>(index)->Bool();
      bool gate = m_note_on >= 0;
      BypassEnvelope(!enable, gate);
      break;
    }

    case kParamAttackTime:
    {
      double attack = GetParam<IDoubleExpParam>(index)->Value() * 0.001;
      SetAttackTime(attack);
      break;
    }

    case kParamDecayTime:
    {
      double decay = GetParam<IDoubleExpParam>(index)->Value() * 0.001;
      SetDecayTime(decay);
      break;
    }

    case kParamSustainLevel:
    {
      double sustain = GetParam<IDoubleParam>(index)->DBToAmp();
      SetSustainLevel(sustain);
      break;
    }

    case kParamReleaseTime:
    {
      double release = GetParam<IDoubleExpParam>(index)->Value() * 0.001;
      SetReleaseTime(release);
      break;
    }

    case kParamCutoffFrequency:
    {
      double cutoff = GetParam<IDoubleExpParam>(index)->Value();
      SetCutoffFrequency(cutoff);
      break;
    }

    case kParamResonance:
    {
      double resonance = GetParam<IDoubleParam>(index)->Value();
      SetResonance(resonance);
      break;
    }

    case kParamLFOFrequency:
    {
      double rate = GetParam<IDoubleExpParam>(index)->Value();
      SetLFOFrequency(rate);
      break;
    }

    case kParamLFOAmplitude:
    {
      double depth = GetParam<IDoubleParam>(index)->Value();
      SetLFOAmplitude(depth);
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
      m_synth->Attack();
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
  #ifdef WDL_DENORMAL_FTZMODE
  WDL_denormal_ftz_scope denormalFtz;
  #endif

  bool pluginIsBypassed = IsBypassed() || GetParam<IBoolParam>(kParamBypass)->Bool();
  bool envelopIsEnabled = !m_synth->EnvelopeIsBypassed();

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
    bool gate = !pluginIsBypassed && (m_note_on >= 0 || envelopIsEnabled);
    Process(&outputs[0][offset], block, gate);

    offset = next;
  }

  memcpy(outputs[1], outputs[0], samples * sizeof(double));

  m_midi_queue.Flush(samples);
}

bool DrMixAISynth::OnGUIRescale(int wantScale)
{
	// Load image set depending on host GUI DPI.
	IGraphics* pGraphics = GetGUI();
	pGraphics->Rescale(wantScale);

	typedef IGraphics::BitmapResource Resource;

	// Half scale image set.
	static const Resource halfScaleImageSet[] =
	{
		Resource(IPLUG_RESOURCE(BACKGROUND_PNG)),
		Resource(IPLUG_RESOURCE(SWITCH_PNG)),
		Resource(IPLUG_RESOURCE(KNOB_PNG)),
		Resource()
	};

	// Full scale image set.
	static const Resource fullScaleImageSet[] =
	{
		Resource(IPLUG_RESOURCE(BACKGROUND_2X_PNG)),
		Resource(IPLUG_RESOURCE(SWITCH_2X_PNG)),
		Resource(IPLUG_RESOURCE(KNOB_2X_PNG)),
		Resource()
	};

	const Resource* pResources = halfScaleImageSet;
	if (wantScale == IGraphics::kScaleFull) pResources = fullScaleImageSet;

	pGraphics->LoadBitmapResources(pResources);

	return true;
}
