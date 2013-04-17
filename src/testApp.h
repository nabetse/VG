#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxHttpUtils.h"

class testApp : public ofBaseApp{

	public:
		//GUI
		ofTexture				blackBrd;
		unsigned char         * rgbaSalidaColorPx;

		struct brush {
			ofImage				mask;
			unsigned char     * maskPx;
			int					w, h;
		};

		brush					brocha;
		float					posBx, posBy;

		//CV
		int						camW,camH, BBw, BBh, colorTol;
		int						findHue;
		ofColor					elColor;
		ofVideoGrabber			camara;
		ofxCvColorImage			video, hsb;
		ofxCvGrayscaleImage		hue,sat,bri,filtered;    
		ofxCvContourFinder		contours;

		//HTTP

		ofxHttpUtils httpUtils;
        int counterMouse, counter;
        string responseStr;
        string requestStr;
        string action_url;
		void newResponse(ofxHttpResponse & response);
   

		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);	
		
};
