#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxOsc.h"
#include "pkmGaborAtom.h"

#define RECEIVE_PORT 12345
#define MS_PER_STIMULI 500
#define USE_OSC

class testApp : public ofBaseApp
{

public:
    void setup                  ();
    void update                 ();
    void draw                   ();
    void exit                   ();

    void keyPressed             (int key);
    void keyReleased            (int key);
    void mouseMoved             (int x, int y );
    void mouseDragged           (int x, int y, int button);
    void mousePressed           (int x, int y, int button);
    void mouseReleased          (int x, int y, int button);
    void windowResized          (int w, int h);
    void dragEvent              (ofDragInfo dragInfo);
    void gotMessage             (ofMessage msg);
    
private:
    ofShader                    noiseShader;
    pkmGaborAtom                atom;
    int                         screenWidth, 
                                screenHeight;
    int                         kernelSize;
    float                       sigma, 
                                theta, 
                                freq, 
                                psi;
    vector<float>               sigmas, thetas, freqs, psis;
    vector<vector<int> >        lut;
    int                         currentLut;
    int                         currentTrial;
    
    ofxOscReceiver              oscReceiver;
    ofstream                    outfile;
    
    bool                        bNoiseState, bFirstTrial;
};
