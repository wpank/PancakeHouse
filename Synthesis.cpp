#include "Synthesis.h"
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

enum EParams
{
  kNumParams
};

enum ELayout
{
	kWidth = GUI_WIDTH,
	kHeight = GUI_HEIGHT,
	kKeybX = 1,
	kKeybY = 0
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
	lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1)
{
  TRACE;

  
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  //                           C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octives: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);

  pGraphics->AttachControl(mVirtualKeyboard);
  
  AttachGraphics(pGraphics);
  CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &Synthesis::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &Synthesis::onNoteOff);
}

Synthesis::~Synthesis() {}

void Synthesis::ProcessDoubleReplacing(double** inputs, 
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
		if (velocity > 0) {
			mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
			mOscillator.setMuted(false);
		}
		else {
			mOscillator.setMuted(true);
		}
		//leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * velocity / 127.0;
		/* 
		if (mEnvelopeGenerator.getCurrentStage() == EnvelopeGenerator::ENVELOPE_STAGE_OFF) {
			mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
		}
		if (mEnvelopeGenerator.getCurrentStage() == EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN) {
			mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
		} 
		*/
		leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0;
	}
	mMIDIReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelopeGenerator.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

}

void Synthesis::CreatePresets() {
	
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
	mVirtualKeyboard->SetDirty();
}

void Synthesis::processVirtualKeyboard() {
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
