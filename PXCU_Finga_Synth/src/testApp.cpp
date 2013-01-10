//
//  fingaSynth PXCU example
//
//
//  created by Rui Pereira(rux) for MadSci-Labs @HAVASWW 1/10/2013.
//

#include "testApp.h"

PXCGesture::GeoNode::Label gFingers[] = {	PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_THUMB,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_INDEX,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_MIDDLE,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_RING,
											PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY|PXCGesture::GeoNode::LABEL_FINGER_PINKY
};


PXCGesture::GeoNode::Label gHands[] = {PXCGesture::GeoNode::LABEL_BODY_HAND_PRIMARY,PXCGesture::GeoNode::LABEL_BODY_HAND_SECONDARY};

ofPoint screenSize;
float screenScale;

int baseFreq;
float modulatorVolume;
bool bSynth = false;


//--------------------------------------------------------------
void testApp::setup()
{
	ofHideCursor();

	mSession = PXCUPipeline_Init((PXCUPipeline)PXCU_PIPELINE_GESTURE);
	
	if(!mSession) ofExit();
	
	if(PXCUPipeline_QueryLabelMapSize(mSession, &mlw, &mlh)){
		mLabelMap = new unsigned char[mlw*mlh];
		mLabelTex = ofTexture();
		mLabelTex.allocate(mlw,mlh,GL_LUMINANCE);
	}

	// - - - - - - - - - - - - - - - - audio synth stuff setup
	int bufferSize = BUFFERSIZE;
	sampleRate = 44100;

	soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);

	baseFreq = 440;

	for(int i=0;i<NFINGERS;++i){	
		// setup finger stuff
		fingers[i].bExists = false;
		fingers[i].geoLabel = gFingers[i];
		fingers[i].pos.set(0,0,0);

		// setup carrier oscillators - synth stuff
		fingers[i].phase = 0.0;
		fingers[i].frequency = 0;		
		fingers[i].phaseAdder = (fingers[i].frequency/sampleRate)*TWO_PI;
	}

	volume = 1.0 / NFINGERS;

	for(int i=0; i< 2; i++){
		//setup hands stuff
		hands[i].bExists = false;
		hands[i].geoLabel = gHands[i];
		hands[i].pos.set(0,0,0);

		//setup modulator oscillator - synth stuff
		hands[i].frequency = 0.0;
		hands[i].phase = 0.0;
		hands[i].phaseAdder = (hands[i].frequency/sampleRate)*TWO_PI;

	}


	screenScale = 4.0; // some scalar for screen positioning // not accurate

	ofEnableAlphaBlending();

	ofBackground(0);
	FBO32.allocate(ofGetScreenWidth(),ofGetScreenHeight(), GL_RGBA32F_ARB);
	
	FBO32.begin();
		ofClear(0,0,0,0);
	FBO32.end();



}
  
//--------------------------------------------------------------
void testApp::update()
{
	if(PXCUPipeline_AcquireFrame(mSession, true))
	{
		if(PXCUPipeline_QueryLabelMap(mSession, mLabelMap, 0))
		{
			mLabelTex.loadData(mLabelMap,mlw,mlh,GL_LUMINANCE);
			int nFingersDetected = 0;

			for(int i=0;i<NFINGERS;++i)
			{
				fingers[i].bExists = PXCUPipeline_QueryGeoNode(mSession, fingers[i].geoLabel, &mNode);
	
					if(fingers[i].bExists ) {	

						cout<< "finger " << fingers[i].geoLabel << " confidence==" << mNode.confidence<<endl;


					nFingersDetected ++;
					
					fingers[i].pos.set(mlw - mNode.positionImage.x, mNode.positionImage.y, mNode.positionWorld.z);

					// set new frequency and recalculate phase adder/step
					baseFreq = ofMap(fingers[i].pos.y, 0, mlh, 180, 720, true);

					//fingers[i].frequency = baseFreq + (i*baseFreq);  //use this instead for harmonics http://en.wikipedia.org/wiki/Harmonic
					fingers[i].frequency = baseFreq*5 - (i*(baseFreq*.5));

					if(i==0) fingers[i].frequency = baseFreq;
					fingers[i].phaseAdder = (fingers[i].frequency/sampleRate)*TWO_PI;

					// ribbon stuff
					if(fingers[i].pts.size() >= RIBBONSIZE){
						fingers[i].pts.erase(fingers[i].pts.begin());
					}  
					ofPoint temp;
					temp.set(fingers[i].pos*screenScale);
					fingers[i].pts.push_back(temp);
				
				} else {
					fingers[i].pts.clear();
				} 

			}

			if(nFingersDetected==0){ 
				bSynth = false;
			} else {
				bSynth = true;
			}

			for(int i=0; i <2; i++){
				hands[i].bExists = PXCUPipeline_QueryGeoNode(mSession, hands[i].geoLabel, &mNode);
				if(hands[i].bExists){
					hands[i].pos.set(mlw- mNode.positionImage.x, mNode.positionImage.y, mNode.openness*.01);
					if(i==1){
						hands[i].frequency = ofMap(mNode.positionImage.x, 0, mlh, -3, 20, true);
						hands[i].phaseAdder = (hands[i].frequency/sampleRate)*TWO_PI;
					}
				}
				
			}
		}


		PXCUPipeline_ReleaseFrame(mSession);
	}
}

//--------------------------------------------------------------
void testApp::draw()
{	
	ofSetColor(255);
	mLabelTex.draw(ofGetScreenWidth(), 0, -mlw, mlh );
	ofPushStyle();

	ofEnableBlendMode(OF_BLENDMODE_ADD);

	for(int i=0;i<NFINGERS;i++)
	{
		if(fingers[i].bExists)
		{
			if(i==0) ofSetColor(255,0,0);
			if(i==1) ofSetColor(0,255,0);
			if(i==2) ofSetColor(0,0,255);
			if(i==3) ofSetColor(255,0,255);
			if(i==4) ofSetColor(0,255,255);

			//ofSetColor(255,255,0);
			ofFill();
			ofCircle((ofGetScreenWidth()-mlw) +fingers[i].pos.x, fingers[i].pos.y, 5);
			//ofNoFill();
			ofCircle(fingers[i].pos*screenScale, 20);
		}
	}

	for(int i = 0; i<2; i++){
		if(hands[i].bExists){
			ofSetColor(255);
			ofFill();
			if(i==0)	ofNoFill();
			ofCircle((hands[i].pos*screenScale)+ hands[i].pos.z*.25, (hands[i].pos.z*100)*.5);
		}
	}
	ofPopStyle();

	FBO32.begin();
	fadeAmnt=4;
	ofSetColor(255,255,255,fadeAmnt);
	ofRect(0,0,ofGetScreenWidth(),ofGetScreenHeight());



	for(int i = 0; i <NFINGERS ; i++){
		if(fingers[i].bExists){
			ofFill();
			ofSetColor(255-(i*30));

			// ribbon
			if(i==0) ofSetColor(255,0,0);
			if(i==1) ofSetColor(0,255,0);
			if(i==2) ofSetColor(0,0,255);
			if(i==3) ofSetColor(255,0,255);
			if(i==4) ofSetColor(0,255,255);

			ofNoFill();
			ofBeginShape();
			for(int j=0; j< fingers[i].pts.size(); j++){
				ofVertex(fingers[i].pts[j].x, fingers[i].pts[j].y);
			}
			ofEndShape(false);
		}
	}

	FBO32.end();


	ofSetColor(255,255,255);
	FBO32.draw(0,0);


	ofSetColor(255,150);
	string info = "";
	for(int i = 0; i< NFINGERS; i++){
		info += "finger>"+ofToString(i) + " freq::" + ofToString(fingers[i].frequency) + "\n";
	}

	ofDrawBitmapString(info, ofGetScreenWidth()-200, ofGetHeight()-300); 
	// draw waveform
	ofFill();
	ofSetColor(255,140);
	ofBeginShape();
	ofVertex(0,0);


		for(int i =0; i <BUFFERSIZE; i++){
			float y = ofMap(i, 0, 512, 0, ofGetHeight(),true);
			if(bSynth){
				ofVertex(200+(audioGrafx[i]*200), y);
			} else {
				audioGrafx[i] *= .9;
				ofVertex(200+(audioGrafx[i]*200), y);
			}

		}

	ofVertex(0,ofGetHeight());
	ofEndShape(false);


}

// -------------------------------------
void testApp::audioOut(float *output, int bufferSize, int nChannels){
	float sample;
	
	for (int i = 0; i < NFINGERS; i++){
        while(fingers[i].phase > TWO_PI){
            fingers[i].phase -= TWO_PI;
        }
    }
        while(hands[1].phase > TWO_PI){
            hands[1].phase -= TWO_PI;
        }	


	for(int i =0; i< bufferSize; i++){	
		for(int f=0; f<NFINGERS; f++){
			if(fingers[f].bExists){
				float carrierFrequency[5];
				float modVal = ofMap(hands[1].pos.y, 0, mlh, 0,hands[1].pos.x*screenScale);
				
				carrierFrequency[f] = ofMap(sin(hands[1].phase),-1,1,fingers[f].frequency-(modVal*hands[1].pos.z), fingers[f].frequency+(modVal*hands[1].pos.z)); // multiply modVal by openness scalar to control FM amplitude
				
				fingers[f].phaseAdder = (carrierFrequency[f]/44100)*TWO_PI;

				hands[1].phase += hands[1].phaseAdder;

				fingers[f].phase += fingers[f].phaseAdder;
				  
				sample = sin(fingers[f].phase) * ofMap(sin(hands[1].phase), -1,1,0,1,true);  // A M 
				
				if(f==0){
                    audioGrafx[i] = output[i* nChannels + 0 ] = sample * volume;
                    output[i* nChannels + 1 ] = sample * volume;
                } else {
                    audioGrafx[i] = output[i* nChannels + 0 ] += sample * volume;
                    output[i* nChannels + 1 ] += sample * volume;
                }
			}
		}
	}
	

}