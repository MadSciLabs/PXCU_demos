//
//  fingaSynth PXCU example
//
//  
//  created by Rui Pereira(rux) for MadSci-Labs @HAVASWW 1/10/2013.
//

#pragma once

#include "ofMain.h"
#include "pxcupipeline.h"

#define NFINGERS 5
#define RIBBONSIZE 256
#define BUFFERSIZE 512

struct finger {
	PXCGesture::GeoNode::Label geoLabel;
	ofPoint pos;
	bool	bExists;

	// synth stuff
	float frequency;
	float phase;
	float phaseAdder;

	// ribbons stuff
	vector<ofPoint> pts;

};

struct hand {
	PXCGesture::GeoNode::Label geoLabel;
	ofPoint pos;
	bool bExists;

	// modulator synth stuff
	float frequency;
	float phase;
	float phaseAdder;
};

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		// audio synth stuff
		void audioOut(float * input, int bufferSize, int nChannels);
		ofSoundStream soundStream;
		float sampleRate;
		float volume;

		// PXCU stuff
		int mlw,mlh;
		unsigned char *mLabelMap;
		ofTexture mLabelTex;
		PXCUPipeline_Instance mSession;
		PXCGesture::GeoNode mNode;

		finger	fingers[NFINGERS];
		hand	hands[2];


		// FBO stuff

		ofFbo FBO32;
		int fadeAmnt;

		// sound synth grafx stuff
		float audioGrafx[BUFFERSIZE];

		
};
