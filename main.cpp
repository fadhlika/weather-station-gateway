#include <wiringPiSPI.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <time.h> 
#include <stdlib.h>
#include <LoRa.h>
#include <raspicam/raspicam_cv.h>

using namespace std; 

raspicam::RaspiCam_Cv Camera;
 
void onReceive(int packetSize);

int main(void){
	wiringPiSetup();

	LoRa.setPins(0, 5, 6);
	if(!LoRa.begin(433E6)){
		printf("Starting LoRa failed!\n");
		exit(EXIT_FAILURE);
	}

	LoRa.onReceive(&onReceive);

	LoRa.receive();

	Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	Camera.set(CV_CAP_PROP_EXPOSURE, 100);
	while(1) {
		//int packetSize = LoRa.parsePacket();
		//if(packetSize) onReceive(packetSize);
		delay(3);
	}
	
	return 0;
}

void onReceive(int packetSize){
	time_t rawtime;
  	struct tm * timeinfo;

  	time (&rawtime);
	timeinfo = localtime (&rawtime);
	
	std::ostringstream os;
	ofstream file_data;
	file_data.open("data/aviana.txt", std::ios_base::app);
	// received a packet
  	os << timeinfo->tm_year + 1900 << "-" << std::setfill('0') << std::setw(2) << timeinfo->tm_mon + 1 << "-" << std::setfill('0') << std::setw(2) << timeinfo->tm_mday
		<< "T" << std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_min << ":" << std::setfill('0') << std::setw(2) << timeinfo->tm_sec << ";";
	      
  	// read packet
  	for (int i = 0; i < packetSize; i++) {
    		os << (char)LoRa.read();
  	}

  	// print RSSI of packet
  	os << ";" << LoRa.packetRssi() << std::endl;

	std::cout << os.str();
	
	if(packetSize < 10) return;

	file_data << os.str();

	cv::Mat image;
	if (!Camera.open()) {printf("Error opening the camera\n");return;}
	Camera.grab();
	Camera.retrieve (image);
	Camera.release();

	char filename[100];
	sprintf(filename, "davis/%d-%.2d-%.2dT%.2d-%.2d-%.2d.jpg",
			timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	cv::imwrite(filename, image);
}
