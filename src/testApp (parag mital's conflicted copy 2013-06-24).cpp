#include "testApp.h"

string getNextFilename(string basename, string extension)
{
	string filename;
	int fileno = 1;
	ifstream in;
	do {
		in.close();
		stringstream str;
		// format the filename here
		if(fileno == 1)
            str << basename << "." << extension;
		else
            str << basename << "(" << (fileno-1) << ")." << extension;
		filename = str.str();
		++fileno; 
		// attempt to open for read
		in.open( ofToDataPath(filename).c_str() );
	} while( in.is_open() );
	in.close();	
    return filename;
}


void testApp::exit(){
    outfile.close();
}

//--------------------------------------------------------------
void testApp::setup(){
    screenWidth         = 1024;
    screenHeight        = 768;
    kernelSize          = screenWidth / 2;
    currentLut          = 0;
    currentTrial        = 0;
    bNoiseState         = true;
    bFirstTrial         = true;
    
//    static const float sigmas[] = {1.5, 1.25, 1.0, 0.75, 0.5, 0.25, 0.125, 0.0625};
//    static const float thetas[] = {0, 45, 90, 135, 180, 225, 270, 315};
//    static const float freqs[] = {0.0625, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 1.0};
    
    static const float sigmas[] = {1.25, 0.125, 0.25, 1.5, 1.0, 0.5, 0.0625, 0.75};
    static const float thetas[] = {0, 45, 225, 270, 135, 180, 315, 90};
    static const float freqs[] = {0.375, 0.0625, 0.625, 0.5, 1.0, 0.125, 0.75, 0.25};

    
    this->sigmas = vector<float>(sigmas, sigmas + sizeof(sigmas) / sizeof(sigmas[0]) );
    this->thetas = vector<float>(thetas, thetas + sizeof(thetas) / sizeof(thetas[0]) );
    this->freqs = vector<float>(freqs, freqs + sizeof(freqs) / sizeof(freqs[0]) );
    
    for (int i = 0; i < this->sigmas.size(); i++) {
        for (int j = 0; j < this->thetas.size(); j++) {
            for (int k = 0; k < this->freqs.size(); k++) {
                int lut[] = {i, j, k};
                vector<int> clut(lut, lut + sizeof(lut) / sizeof(lut[0]));
                this->lut.push_back(clut);
            }
        }
    }
    
    	//////////////////////////////////////////////////////
	// now create the file so that we can start adding frames to it:
    string filename = getNextFilename("eeg-record-trial-" + ofToString(currentTrial), "txt");
	outfile.open( ofToDataPath(filename).c_str() );
    
    oscReceiver.setup(RECEIVE_PORT);
    atom.allocate(max(screenWidth, screenHeight));
    noiseShader.load(ofToDataPath("noise"));
    ofSetVerticalSync(true);
    ofSetWindowShape(screenWidth, screenHeight);
    ofResetElapsedTimeCounter();
}

//--------------------------------------------------------------
void testApp::update(){
    /*
    sigma = 2.0 * kernelSize * sinf(ofGetElapsedTimef() / 12.0) / 5.0;
    theta = 180.0 * cosf(ofGetElapsedTimef() / 6.0);
    freq = sinf(ofGetElapsedTimef() / 2.0);
     */
    
    if (!bFirstTrial && ofGetElapsedTimeMillis() >= MS_PER_STIMULI) {
        ofResetElapsedTimeCounter();
        bNoiseState = !bNoiseState;
        if (bNoiseState) {
            currentLut++;
            if (currentLut == 512) {
                currentLut = 0;
                currentTrial++;
                if (currentTrial == 10) {
                    cout << "[OK] Finished!" << endl;
                    outfile.close();
                    OF_EXIT_APP(0);
                }
                outfile.close();
                string filename = getNextFilename("eeg-record-trial-" + ofToString(currentTrial), "txt");
                outfile.open( ofToDataPath(filename).c_str() );
            }
        }
    }
    
    if (bNoiseState) {
        
    }
    else {
        sigma = kernelSize * sigmas[lut[currentLut][0]];
        theta = thetas[lut[currentLut][1]];
        freq = freqs[lut[currentLut][2]];
        atom.update(sigma, theta, freq);
    }
    
    string delimiter = ",";
#ifdef USE_OSC
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage m;
        if(oscReceiver.getNextMessage(&m))
        {
            if (m.getAddress() == "/eeg") {
#endif
                outfile << ofGetTimestampString() 
                << delimiter << bNoiseState 
                << delimiter << currentTrial 
                << delimiter << currentLut 
                << delimiter << sigma 
                << delimiter << theta 
                << delimiter << freq;
#ifdef USE_OSC
                for (int ch = 0; ch < 22; ch++) {
                    outfile << delimiter << m.getArgAsFloat(ch);
                }
#endif
                outfile << delimiter;
                outfile << endl;
                
#ifdef USE_OSC
            }
        }
    }
#endif
    
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    if (bNoiseState) {
        
        noiseShader.begin();
        noiseShader.setUniform1f("time", ofGetElapsedTimef());
        atom.draw(0, 0, screenWidth, screenHeight);
        noiseShader.end();
                
        /*
        ofFill();
        float res = 7.0;
        for (float x = 0; x < ofGetWidth(); x+=res) {
            for (float y = 0; y < ofGetHeight(); y+=res) {
                ofSetColor(rand() % 255);
                ofRect(x, y, res, res);
            }
        }
        */
    }
    else {
        ofSetColor(255);
        atom.draw(0, 0, screenWidth, screenHeight);
        float halfScreenWidth = screenWidth / 2.0;
        float halfScreenHeight = screenHeight / 2.0;
        ofLine(halfScreenWidth, halfScreenHeight - 5, halfScreenWidth, halfScreenHeight + 5);
        ofLine(halfScreenWidth - 5, halfScreenHeight, halfScreenWidth + 5, halfScreenHeight);
        char buf[256];
        sprintf(buf, "%f", ofGetFrameRate());
        ofSetWindowTitle(buf);
        sprintf(buf, "trial: %02d, iter: %03d, sigma: %01.4f, theta: %03.0f, freq: %04.4f", currentTrial, currentLut, sigma, theta, freq);
        //ofDrawBitmapString(buf, 20, 20);
        cout << buf << endl;
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    if(key == 'f')
        ofToggleFullscreen();
    else if(key == 'm') 
        atom.toggleMultiple();
    else if(key == ' ')
        bFirstTrial = false;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}