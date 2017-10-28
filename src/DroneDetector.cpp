#include "DroneDetector.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
#include <ctime>

double oldPsi;
double oldTime;

using namespace cv;

DroneDetector::DroneLocation DroneDetector::GetLocation(std::vector< std::vector<Point> > contours) {
    std::vector<Point2f> leds(contours.size());
    float xSum = 0;
    float ySum = 0;

    // Calculate weighted average of contours
    for(unsigned int i = 0; i < contours.size(); i++ ) {
	float radius;
	minEnclosingCircle((Mat)contours[i], leds[i], radius);
	xSum += leds[i].x;
	ySum += leds[i].y;
    }
    double xAvg = xSum / contours.size();
    double yAvg = ySum / contours.size();
    Point2f avg(xAvg, yAvg);

    // Find the two outermost LEDs 
    std::vector<Point2f> outerLeds(2);
    double max1 = 0;
    double max2 = 0;
    for(unsigned int i = 0; i < leds.size(); i++) {
	double max = norm(Mat(avg), Mat(leds[i]));
	if (max > max1 && max1 <= max2) {
	    outerLeds[0] = leds[i];
	    max1 = max;
	} 
	else if (max > max2 && max2 < max1) {
	    outerLeds[1] = leds[i];
	    max2 = max;
	}
    }
    Point2f center = (outerLeds[0] + outerLeds[1]) / 2;
    Point2f direction = avg - center;

    double PIXELS2METERS = 8.0/1080.0;
    double X_OFF = 7.0;
    double Y_OFF = 3.8;

    Point2f offset(X_OFF, Y_OFF);
    Point2f physicalCenter = center * PIXELS2METERS - offset;

    Vec4f line;
    fitLine(leds, line, CV_DIST_L2, 0, 0.01, 0.01);

    Point2f lineDirection = Point2f(line[0], line[1]);
    double psi;
    if(direction.dot(lineDirection) > 0)
	psi = atan2(line[1], line[0]);
    else
	psi = atan2(-line[1], -line[0]);

    Point2f dir(cos(psi), sin(psi));

    // Offset for mounting
    psi = fmod(psi + (M_PI/2), 2*M_PI);
    double time = std::clock() / (double) CLOCKS_PER_SEC;
    if (time < oldTime + 1 && abs(psi-oldPsi) > 0.35 && abs(psi-oldPsi) < 5.9) {
	psi = oldPsi;
	std::cout << "========== PSI INVALID ========\n";
    }
    else {
	oldPsi = psi;
	oldTime = time;
    }

    std::cout << "x: " << physicalCenter.x << ", y: " << physicalCenter.y << "psi: " << (psi / M_PI * 180) << "\n";
    DroneLocation loc;
    loc.deltaIntensity = 0;
    loc.x = -physicalCenter.y;
    loc.y = physicalCenter.x;
    loc.psi = psi;
    
    // According to conventions
    return loc; 
}

DroneDetector::DroneLocation DroneDetector::FindDrones(Mat frame) {
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    Mat thresh;
    
    threshold(frame, thresh, 100, 255, CV_THRESH_BINARY);
    findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    std::cout << "Found " << contours.size() << " contours\n";
    if (contours.size() == 4) {
	return GetLocation(contours);
    }
    else {
	DroneLocation loc;
        if(contours.size() > 4) {
            loc.deltaIntensity = -100;
        } 
        else {
            loc.deltaIntensity = 100; 
        }
	return loc;
    }
}
