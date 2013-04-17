#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	//Screen
	ofSetFrameRate(60);
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetWindowTitle("videoGrafitti");
	ofSetVerticalSync(true);
	int screenW = ofGetScreenWidth();
	int screenH = ofGetScreenHeight();
	ofSetWindowShape(screenW, screenH);
	ofSetWindowPosition(0, 0);

	ofBackground(0,30,0);
    
	//VG
	BBw = 640;
	BBh = 480;
    camW = 320;
    camH = 240;
	colorTol = 7;
	camara.listDevices();
	//camara.setDeviceID(1);
    camara.initGrabber(camW, camH, true);
    
    //reserve memory for cv images
    video.allocate(camW, camH);
    hsb.allocate(camW, camH);
    hue.allocate(camW, camH);
    sat.allocate(camW, camH);
    bri.allocate(camW, camH);
    filtered.allocate(camW, camH);

	brocha.mask.loadImage("brochas/brocha.png");
	brocha.mask.setImageType(OF_IMAGE_GRAYSCALE); // clave para bajarlo a 8byte de profundidad
	brocha.w = brocha.mask.width;
	brocha.h = brocha.mask.height;
	brocha.maskPx = new unsigned char [brocha.w*brocha.h];
	brocha.maskPx = brocha.mask.getPixels();

	blackBrd.allocate(BBw, BBh, GL_RGBA);
	rgbaSalidaColorPx = new unsigned char [BBw*BBh*4];
	for (int i = 0; i < BBw; i++){
		for (int j = 0; j < BBh; j++){
			int pos = (j * BBw + i);
			rgbaSalidaColorPx[pos*4  ] = 0;
			rgbaSalidaColorPx[pos*4+1] = 0;
			rgbaSalidaColorPx[pos*4+2] = 0;
			rgbaSalidaColorPx[pos*4+3] = 0;  
		}
	}
	blackBrd.loadData(rgbaSalidaColorPx, BBw, BBh, GL_RGBA);

	//HTTP
	ofAddListener(httpUtils.newResponseEvent,this,&testApp::newResponse);
	httpUtils.start();
	
}

//--------------------------------------------------------------
void testApp::update(){
	camara.update();
    
    if (camara.isFrameNew()) {
        video.setFromPixels(camara.getPixels(), camW, camH);
        
        video.mirror(false, true);
        
        hsb = video;
        //convert to hsb
        hsb.convertRgbToHsv();
        
        //store the three channels as grayscale images
        hsb.convertToGrayscalePlanarImages(hue, sat, bri);
        
        //filter image based on the hue value were looking for
        for (int i=0; i<camW*camH; i++) {
            filtered.getPixels()[i] = ofInRange(hue.getPixels()[i],findHue-colorTol,findHue+colorTol) ? 255 : 0;
        }

        filtered.flagImageChanged();
        /* ofxCvContourFinder::findContours( ofxCvGrayscaleImage&  input,
									  int minArea,
									  int maxArea,
									  int nConsidered,
									  bool bFindHoles,
                                      bool bUseApproximation) {*/
		if (contours.findContours(filtered, 5, camW*camH/4, 1, false) > 0){ // si hay algun blob
			
			posBx = ofMap(contours.blobs[0].centroid.x, 0, camW, 0, BBw);
			posBy = ofMap(contours.blobs[0].centroid.y, 0, camH, 0, BBh);

			for (int xx = posBx - (brocha.w/2); xx < posBx + (brocha.w/2); xx++){
				for (int yy = posBy - (brocha.h/2); yy < posBy + (brocha.h/2); yy++){
					if (xx >= 0 && xx < BBw && yy >= 0 && yy < BBh) {
						int pos = (yy * BBw + xx);
						// posicion relativa a brocha
						int xBrocha = (xx - posBx) + (brocha.w/2);
						int yBrocha = (yy - posBy) + (brocha.h/2);
						int posB = (yBrocha * brocha.h + xBrocha);
						// juego de alphas
//							int alphaOriginal = alphaColorPx[pos*4+3];
						int alphaAnterior = rgbaSalidaColorPx[pos*4+3];
						//if (alphaAnterior != alphaOriginal){
						int alphaBrocha = brocha.maskPx[posB];
//								int alphaUno = min(alphaOriginal, alphaBrocha);
							//int newAlpha = max(alphaUno, alphaAnterior);
						int newAlpha = max(alphaBrocha, alphaAnterior);
							// pintame
						rgbaSalidaColorPx[pos*4] = elColor.r;
						rgbaSalidaColorPx[pos*4+1] = elColor.g;
						rgbaSalidaColorPx[pos*4+2] = elColor.b;
						rgbaSalidaColorPx[pos*4+3] = newAlpha;
							//if (newAlpha == alphaOriginal) puntosLLenados++;
						//}
					}
				}
			} // ya dibujo la brocha

			blackBrd.loadData(rgbaSalidaColorPx, BBw, BBh, GL_RGBA);//GL_LUMINANCE_ALPHA);
			
		}




	}
}

//--------------------------------------------------------------
void testApp::draw(){
	ofBackground(150);
	ofSetColor(255,255,255);
    
    //draw all cv images
    video.draw(0,0);
    hue.draw(0,240);
	/*
    hsb.draw(0,240);
    hue.draw(0,240);
    sat.draw(320,240);
    bri.draw(640,240);
	*/
    filtered.draw(0,480);
    contours.draw(0,480);
    
    ofSetColor(255, 0, 0);
    ofFill();
    
	//ofR

    //draw red circles for found blobs
    for (int i=0; i<contours.nBlobs; i++) {
        ofCircle(contours.blobs[i].centroid.x, contours.blobs[i].centroid.y, 20);
    } 

	ofSetColor(255,255,255);
	ofEnableAlphaBlending();
	blackBrd.draw(350, 20);
	ofDisableAlphaBlending();

	ofSetColor(elColor);
	ofRect(350, 500, 100, 100);

	ofSetColor(255);
	ofDrawBitmapString(requestStr,350,620);
	ofDrawBitmapString(responseStr,350,660);
}

void testApp::newResponse(ofxHttpResponse & response){
	string laRespuesta = (string)response.responseBody;
	ofStringReplace(laRespuesta, "#", "0x");
	responseStr = ofToString(response.status) + ": " + laRespuesta;
	cout << "Este es el mensaje " << laRespuesta << endl;

	std::stringstream str;
//	string s1 = "5f0066
	str << laRespuesta;
	int value;
	str >> std::hex >> value;
	elColor.setHex(value);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	/*
	case (key=='b' || key=='B') :
		return;
		*/
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
	if (x<camW && y < camH){
		int mx = x;
		int my = y;
    
		findHue = hue.getPixels()[my*camW+mx];
		int posRGB = ((my*camW+mx) * 3);
		elColor = ofColor(video.getPixels()[posRGB], video.getPixels()[posRGB + 1], video.getPixels()[posRGB+2]);
	} else {
		ofxHttpForm form;
		form.action = "http://lamariamedia.com/variables.php";
		form.method = OFX_HTTP_POST;
		form.addFormField("pido", "color");
		string mensaje = "(" + ofToString(x) + "," + ofToString(y) + ")";
		form.addFormField("coord", mensaje);
		httpUtils.addForm(form);
		requestStr = "pido color";	
	}
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