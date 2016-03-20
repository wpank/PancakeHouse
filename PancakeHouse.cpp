#include "PancakeHouse.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmain"
#include "IPlug_include_in_plug_src.h"
#pragma clang diagnostic pop
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;
const double parameterStep = 0.001;
enum EParams
{
	// Oscillator Section:
	mOsc1Waveform = 0,
	mOsc1PitchMod,
	mOsc2Waveform,
	mOsc2PitchMod,
	mOscMix,
	// Filter Section:
	mFilterMode,
	mFilterCutoff,
	mFilterResonance,
	mFilterLfoAmount,
	mFilterEnvAmount,
	// LFO:
	mLFOWaveform,
	mLFOFrequency,
	// Volume Envelope
	mVolumeEnvAttack,
	mVolumeEnvDecay,
	mVolumeEnvSustain,
	mVolumeEnvRelease,
	// Filter envelope:
	mFilterEnvAttack,
	mFilterEnvDecay,
	mFilterEnvSustain,
	mFilterEnvRelease,
	kNumParams
};

typedef struct {
	const char* name;
	const int x;
	const int y;
	const double defaultVal;
	const double minVal;
	const double maxVal;
} parameterProperties_struct;

parameterProperties_struct parameterProperties[kNumParams] =

{
	{ "Osc 1 Waveform", 30, 75, 0, 0, 0 },
	{ "Osc 1 Pitch Mod", 85, 75, 0.0, 0.0, 1.0 },
	{ "Osc 2 Waveform", 203, 75, 0, 0, 0 },
	{ "Osc 2 Pitch Mod", 260, 75, 0.0, 0.0, 1.0 },
	{ "Osc Mix", 146, 75, 0.5, 0.0, 1.0 },
	{ "Filter Mode", 30, 188, 0, 0, 0 },
	{ "Filter Cutoff", 84, 190, 0.99, 0.0, 0.99 },
	{ "Filter Resonance", 139, 190, 0.0, 0.0, 1.0 },
	{ "Filter LFO Amount", 196, 190, 0.0, 0.0, 1.0 },
	{ "Filter Envelope Amount", 251, 190, 0.0, -1.0, 1.0 },
	{ "LFO Waveform", 30, 298, 0, 0, 0 },
	{ "LFO Frequency", 86, 299, 6.0, 0.01, 30.0 },
	{ "Volume Env Attack", 337, 75, 0.01, 0.01, 10.0 },
	{ "Volume Env Decay", 393, 75, 0.5, 0.01, 15.0 },
	{ "Volume Env Sustain", 448, 75, 0.1, 0.001, 1.0 },
	{ "Volume Env Release", 503, 75, 1.0, 0.01, 15.0 },
	{ "Filter Env Attack", 338, 190, 0.01, 0.01, 10.0 },
	{ "Filter Env Decay", 393, 190, 0.5, 0.01, 15.0 },
	{ "Filter Env Sustain", 448, 190, 0.1, 0.001, 1.0 },
	{ "Filter Env Release", 503, 190, 1.0, 0.01, 15.0 },
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,
	kKeybX = 62,
	kKeybY = 425
};

PancakeHouse::PancakeHouse(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
	lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1),
	filterEnvelopeAmount(0.0),
	lfoFilterModAmount(0.1)
{
  TRACE;

  CreateParams();
  CreateGraphics();
  CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &PancakeHouse::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &PancakeHouse::onNoteOff);

  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &PancakeHouse::onBeganEvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &PancakeHouse::onFinishedEnvelopeCycle);
}

PancakeHouse::~PancakeHouse() {}

void PancakeHouse::ProcessDoubleReplacing(double** inputs, 
	double** outputs, 
	int nFrames)
{
  // Mutex is already locked for us.

	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];

	
	processVirtualKeyboard();

	//copy left buffer into right buffer
	for (int i = 0; i < nFrames; ++i) {
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		double lfoFilterModulation = mLFO.nextSample() * lfoFilterModAmount;
		mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
		
		
		//leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * velocity / 127.0;
		/* 
		if (mEnvelopeGenerator.getCurrentStage() == EnvelopeGenerator::ENVELOPE_STAGE_OFF) {
			mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
		}
		if (mEnvelopeGenerator.getCurrentStage() == EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN) {
			mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
		} 
		*/
		mFilter.setCutoffMod((mFilterEnvelopeGenerator.nextSample() * filterEnvelopeAmount) + lfoFilterModulation);
		leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0);
	}
	mMIDIReceiver.Flush(nFrames);
}

void PancakeHouse::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelopeGenerator.setSampleRate(GetSampleRate());
  mFilterEnvelopeGenerator.setSampleRate(GetSampleRate());
  mLFO.setSampleRate(GetSampleRate());
}

void PancakeHouse::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  

}

void PancakeHouse::CreatePresets() {
	
}

void PancakeHouse::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
	mVirtualKeyboard->SetDirty();
}

void PancakeHouse::processVirtualKeyboard() {
	IKeyboardControl* virtualKeyboard = (IKeyboardControl*)mVirtualKeyboard;
	int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;

	if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
		// The note number has changed from a valid key to something else (valid key or nothing). Release the valid key:
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}

	if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
		// A valid key is presesed that wasn't pressed the previous call. Send a "note on" message to the MIDI receiver:
		IMidiMsg midiMessage;
		midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
		mMIDIReceiver.onMessageReceived(&midiMessage);
	}
		
	lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;

}

void PancakeHouse::CreateParams() {
	for (int i = 0; i < kNumParams; i++) {
		IParam* param = GetParam(i);
		const parameterProperties_struct& properties = parameterProperties[i];
		switch (i) {
			// Enum Parameters:
		case mOsc1Waveform:
		case mOsc2Waveform:
			param->InitEnum(properties.name,
				Oscillator::OSCILLATOR_MODE_SAW,
				Oscillator::kNumOscillatorModes);
			// for VST3
			param->SetDisplayText(0, properties.name);
			break;
		case mLFOWaveform:
			param->InitEnum(properties.name,
				Oscillator::OSCILLATOR_MODE_TRIANGLE,
				Oscillator::kNumOscillatorModes);
			// For VST3
			param->SetDisplayText(0, properties.name);
			break;
		case mFilterMode:
			param->InitEnum(properties.name,
				Filter::FILTER_MODE_LOWPASS,
				Filter::knumFilterModes);
			break;
		//Double Parameters
		default:
			param->InitDouble(properties.name,
				properties.defaultVal,
				properties.minVal,
				properties.maxVal,
				parameterStep);
			break;
		}
	}

	GetParam(mFilterCutoff)->SetShape(2);
	GetParam(mVolumeEnvAttack)->SetShape(3);
	GetParam(mFilterEnvAttack)->SetShape(3);
	GetParam(mVolumeEnvDecay)->SetShape(3);
	GetParam(mFilterEnvDecay)->SetShape(3);
	GetParam(mVolumeEnvSustain)->SetShape(2);
	GetParam(mFilterEnvSustain)->SetShape(2);
	GetParam(mVolumeEnvRelease)->SetShape(3);
	GetParam(mFilterEnvRelease)->SetShape(3);

	for (int i = 0; i < kNumParams; i++) {
		OnParamChange(i);
	}
}

void PancakeHouse::CreateGraphics() {
	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	pGraphics->AttachBackground(BG_ID, BG_FN);

	IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
	IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);
	//                            C#      D#          F#      G#      A#
  	int keyCoordinates[12] = { 0, 10, 17, 30, 35, 52, 61, 68, 79, 85, 97, 102 };
	mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /*octaves: */ 4, &whiteKeyImage, &blackKeyImage, keyCoordinates); 
	pGraphics->AttachControl(mVirtualKeyboard);

	IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
	IBitmap filterModeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);
	IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 127);

	for (int i = 0; i < kNumParams; i++) {
		const parameterProperties_struct& properties = parameterProperties[i];
		IControl* control;
		IBitmap* graphic;
		switch (i) {
			//Switches
		case mOsc1Waveform:
		case mOsc2Waveform:
		case mLFOWaveform:
			graphic = &waveformBitmap;
			control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
			break;
		case mFilterMode:
			graphic = &filterModeBitmap;
			control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
			break;
		// Knobs
		default:
			graphic = &knobBitmap;
			control = new IKnobMultiControl(this, properties.x, properties.y, i, graphic);
			break;
		}
		pGraphics->AttachControl(control);
	}
	AttachGraphics(pGraphics);
}

