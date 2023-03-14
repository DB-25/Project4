#include <iostream>
#include <vector>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

cv::Mat cameraMatrix;
cv::Mat distorstion;

// method to calibrate the camera
void calibrateCamera(std::vector<std::vector<cv::Point2f> > cornerList, std::vector<std::vector<cv::Vec3f> > pointList, cv::Mat frame)
{
    std::vector<cv::Mat> rotation, translation;
    cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
    cameraMatrix.at<double>(0, 2) = frame.cols/2;
    cameraMatrix.at<double>(1, 2) = frame.rows/2;
    cout << "Original camera matrix:" << endl << cameraMatrix << endl;
    //Make vectors to be filled by the calibrate camera function
    distorstion = cv::Mat::zeros(5, 1, CV_64F);
    double err = cv::calibrateCamera(pointList, cornerList, frame.size(), cameraMatrix, distorstion, rotation, translation);
    cout << "New camera matrix: " << endl << cameraMatrix << endl;
    printf("Error is: %f\n", err);
    cout << "Distortion " << endl << distorstion << endl;
}

int main()
{
    cv::VideoCapture cap(0); // Open the default camera
    if (!cap.isOpened()) // Check if we succeeded
    {
        std::cerr << "Failed to open camera" << std::endl;
        return -1;
    }

    cv::namedWindow("Calibration", cv::WINDOW_NORMAL);
    cv::resizeWindow("Calibration", 800, 600);

    cv::Mat frame;
    bool calibrated = false;
    int  numFramesCaptured = 0;
    std::vector<cv::Point2f> corners;
    bool found = false;
    int chessboardRows = 6;
    int chessboardCols = 9;
    std::vector<cv::Vec3f> pointSet;
    std::vector<std::vector<cv::Point2f> > cornerList;
    std::vector<std::vector<cv::Vec3f> > pointList;

    while (true)
    {
        cap >> frame; // Capture a frame
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY); // Convert to grayscale

        found = cv::findChessboardCorners(gray, cv::Size(chessboardCols, chessboardRows), corners); // Find the chessboard corners
        if (found)
        {
            cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001);
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria); // Refine the corner locations
            if(!calibrated){
            cv::drawChessboardCorners(frame, cv::Size(chessboardCols, chessboardRows), corners, found); // Draw the corners on the frame
            }
            // Wait for user input to save the corner locations
            char key = cv::waitKey(1);
            if (key == 's')
            {
                std::cout << "Saved corner locations" << std::endl;
                numFramesCaptured++;
                // print the number of frames captured
                std::cout << "Number of frames captured: " << numFramesCaptured << std::endl;
                if(numFramesCaptured >= 5){
                    calibrated = true;
                    std::cout << "Calibration started" << std::endl;
                    calibrateCamera(cornerList, pointList, frame);
                }
                // Add the corners to the corner list
                cornerList.push_back(corners);
                // Create the point set of 3D positions
                pointSet.clear();
                for (int i = 0; i < chessboardRows; i++)
                {
                    for (int j = 0; j < chessboardCols; j++)
                    {
                        pointSet.push_back(cv::Vec3f(j, i, 0));
                    }
                }
                pointList.push_back(pointSet);
            }
            // if the user presses 'w' it will write the camera matrix and distortion to a text file
            if (key == 'w' && calibrated)
            {
                std::cout << "Writing to file" << std::endl;
                std::ofstream file;
                file.open("calibration.txt");
                // write the camera matrix line by line
                for(int i = 0; i < cameraMatrix.rows; i++){
                    for(int j = 0; j < cameraMatrix.cols; j++){
                        file << cameraMatrix.at<double>(i, j) << endl;
                    }
                }
                // write the distortion line by line
                for(int i = 0; i < distorstion.rows; i++){
                    for(int j = 0; j < distorstion.cols; j++){
                        file << distorstion.at<double>(i, j) << endl;
                    }
                }
                file.close();
            }
            // if the user presses 'r' it will read the camera matrix and distortion from a text file
            if (key == 'r')
            {
                std::cout << "Reading from file" << std::endl;
                std::ifstream file;
                file.open("calibration.txt");
                // read the camera matrix line by line
                for(int i = 0; i < cameraMatrix.rows; i++){
                    for(int j = 0; j < cameraMatrix.cols; j++){
                        file >> cameraMatrix.at<double>(i, j);
                    }
                }
                // read the distortion line by line
                for(int i = 0; i < distorstion.rows; i++){
                    for(int j = 0; j < distorstion.cols; j++){
                        file >> distorstion.at<double>(i, j);
                    }
                }
                file.close();
                calibrated = true;
                // print the camera matrix and distortion
                std::cout << "Camera Matrix" << std::endl;
                std::cout << cameraMatrix << std::endl;
                std::cout << "Distortion" << std::endl;
                std::cout << distorstion << std::endl;
        
            }
        
        }
        cv::imshow("Calibration", frame);

        char key = cv::waitKey(1);
        if (key == 'q') // Exit on escape key
        {
            break;
        }
    }

    cap.release(); // Release the camera

    cv::destroyAllWindows();

    return 0;
}


