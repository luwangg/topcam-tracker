#ifndef DRONEDETECTOR
#define DRONEDETECTOR

#include <opencv2/imgproc.hpp>

class DroneDetector {
    public:
        struct DroneLocation {
            cv::Point2f pos;
            double psi;
        };

        DroneDetector(unsigned int nr_drones);
			
        DroneLocation GetLocation(std::vector<cv::Point2f> leds);
        std::vector<DroneLocation> FindDrones(cv::Mat frame, int* deltaIntensity);

    private:
        double oldPsi;
        double oldTime;
        unsigned int nr_drones;	
        std::vector< std::vector<cv::Point2f> > PartitionPoints(std::vector<cv::Point2f> points); 

};

#endif
