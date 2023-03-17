// main.cpp
//
// Dhruv Kamalesh Kumar
// Yalala Mohit
//
// 03-13-2023

#include <iostream>
#include <vector>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

cv::Mat cameraMatrix;
cv::Mat distorstion;
std::vector<cv::Mat> rotationVector, translationVector;
cv::Mat rotation, translation;

// method to calibrate the camera
void calibrateCamera(std::vector<std::vector<cv::Point2f> > cornerList, std::vector<std::vector<cv::Vec3f> > pointList, cv::Mat frame)
{
    cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
    cameraMatrix.at<double>(0, 2) = frame.cols/2;
    cameraMatrix.at<double>(1, 2) = frame.rows/2;
    cout << "Original camera matrix:" << endl << cameraMatrix << endl;
    //Make vectors to be filled by the calibrate camera function
    distorstion = cv::Mat::zeros(5, 1, CV_64F);
    double err = cv::calibrateCamera(pointList, cornerList, frame.size(), cameraMatrix, distorstion, rotationVector, translationVector);
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
    bool extension = false;
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
        if(frame.empty()){
            std::cerr << "Failed to capture frame" << std::endl;
            return -1;
        }
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY); // Convert to grayscale

        // Create the point set of 3D positions
        pointSet.clear();
          for(int i = 0; i > -chessboardRows; i--) {
            for(int j = 0; j < chessboardCols; j++) {
                pointSet.push_back(cv::Point3f(j, i, 0));
            }
        }

        found = cv::findChessboardCorners(gray, cv::Size(chessboardCols, chessboardRows), corners); // Find the chessboard corners
        if (found)
        {
            cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001);
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria); // Refine the corner locations
            if(!calibrated){
            cv::drawChessboardCorners(frame, cv::Size(chessboardCols, chessboardRows), corners, found); // Draw the corners on the frame
            }
            else{
                // since the camera is calibrated
                // we first calculate the position of camera in the world
                // we use cv::solvePnP to get the rotation and translation matrices
                rotation = cv::Mat::zeros(3, 1, CV_64FC1);
                translation = cv::Mat::zeros(3, 1, CV_64FC1);
                bool result = cv::solvePnP(pointSet, corners, cameraMatrix, distorstion, rotation, translation);
                if(result){
                    // print the rotation and translation matrices
                    std::cout << "Rotation: " << rotation << std::endl;
                    std::cout << "Translation: " << translation << std::endl;

                    // lets draw the 3D Axes on the frame at the origin
                    // we first create the 3D points for the axes
                    std::vector<cv::Point3f> axes;  
                    axes.push_back(cv::Point3f(0, 0, 0));
                    axes.push_back(cv::Point3f(1, 0, 0));
                    axes.push_back(cv::Point3f(0, 1, 0));
                    axes.push_back(cv::Point3f(0, 0, 1));
                    // we then project the 3D points to 2D points
                    std::vector<cv::Point2f> projectedPoints;
                    cv::projectPoints(axes, rotation, translation, cameraMatrix, distorstion, projectedPoints);
                    // we then draw the arrowed lines
                    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[1], cv::Scalar(0, 0, 255), 2);
                    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[2], cv::Scalar(0, 255, 0), 2);
                    cv::arrowedLine(frame, projectedPoints[0], projectedPoints[3], cv::Scalar(255, 0, 0), 2);
                    if(extension){
                        // lets draw a 3D house on the frame
                        // we first create the 3D points for the house
                        std::vector<cv::Point3f> house;
                        // bottom corners
                        house.push_back(cv::Point3f(1, 0, 0));
                        house.push_back(cv::Point3f(1, -6, 0));
                        house.push_back(cv::Point3f(9, -6, 0));
                        house.push_back(cv::Point3f(9, 0, 0));
                        // top corners
                        house.push_back(cv::Point3f(1, 0, 3));
                        house.push_back(cv::Point3f(1, -6, 3));
                        house.push_back(cv::Point3f(9, -6, 3));
                        house.push_back(cv::Point3f(9, 0, 3));
                        // roof points
                        house.push_back(cv::Point3f(1, -3, 6));
                        house.push_back(cv::Point3f(9, -3, 6));
                        // we then project the 3D points to 2D points
                        std::vector<cv::Point2f> projectedPoints;
                        cv::projectPoints(house, rotation, translation, cameraMatrix, distorstion, projectedPoints);
                        
                        // fill color for the house
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[0], projectedPoints[1], projectedPoints[2], projectedPoints[3]}}, cv::Scalar(255,255,255));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[0], projectedPoints[4], projectedPoints[7], projectedPoints[3]}}, cv::Scalar(204,209,72));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[1], projectedPoints[5], projectedPoints[6], projectedPoints[2]}}, cv::Scalar(204,209,72));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[4], projectedPoints[5], projectedPoints[8]}}, cv::Scalar(204,209,72));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[7], projectedPoints[6], projectedPoints[9]}}, cv::Scalar(204,209,72));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[0], projectedPoints[1], projectedPoints[5], projectedPoints[4],}}, cv::Scalar(211,0,148));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[3], projectedPoints[2], projectedPoints[6], projectedPoints[7],}}, cv::Scalar(211,0,148));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[8], projectedPoints[9], projectedPoints[7], projectedPoints[4],}}, cv::Scalar(211,0,148));
                        cv::fillPoly(frame, std::vector<std::vector<cv::Point>>{std::vector<cv::Point>{projectedPoints[8], projectedPoints[9], projectedPoints[6], projectedPoints[5],}}, cv::Scalar(211,0,148));
                        // we then draw the lines
                        cv::line(frame, projectedPoints[0], projectedPoints[1], cv::Scalar(31, 240, 255), 4);
                        cv::line(frame, projectedPoints[1], projectedPoints[2],  cv::Scalar(31, 240, 255), 4);
                        cv::line(frame, projectedPoints[2], projectedPoints[3],  cv::Scalar(31, 240, 255), 4);
                        cv::line(frame, projectedPoints[3], projectedPoints[0],  cv::Scalar(31, 240, 255), 4);
                        cv::line(frame, projectedPoints[4], projectedPoints[5], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[5], projectedPoints[6], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[6], projectedPoints[7], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[7], projectedPoints[4], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[0], projectedPoints[4], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[1], projectedPoints[5], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[2], projectedPoints[6], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[3], projectedPoints[7], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[8], projectedPoints[9], cv::Scalar(0, 0, 0), 2);
                        // we draw the roof
                        cv::line(frame, projectedPoints[4], projectedPoints[8], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[5], projectedPoints[8], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[6], projectedPoints[9], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[7], projectedPoints[9], cv::Scalar(0, 0, 0), 2);
                        cv::line(frame, projectedPoints[8], projectedPoints[9], cv::Scalar(0, 0, 0), 2);
                    }else{
                    // lets draw a 3D cuboid on the frame
                    // we first create the 3D points for the cuboid
                    std::vector<cv::Point3f> cuboid;
                    // lets eliminate the first column
                    cuboid.push_back(cv::Point3f(1, 0, 0));
                    cuboid.push_back(cv::Point3f(1, -chessboardRows, 0));
                    cuboid.push_back(cv::Point3f(chessboardCols, -chessboardRows, 0));
                    cuboid.push_back(cv::Point3f(chessboardCols, 0, 0));
                    cuboid.push_back(cv::Point3f(1, 0, 1));
                    cuboid.push_back(cv::Point3f(1, -chessboardRows, 1));
                    cuboid.push_back(cv::Point3f(chessboardCols, -chessboardRows, 1));
                    cuboid.push_back(cv::Point3f(chessboardCols, 0, 1));
                    // we then project the 3D points to 2D points
                    std::vector<cv::Point2f> projectedCuboid;
                    cv::projectPoints(cuboid, rotation, translation, cameraMatrix, distorstion, projectedCuboid);
                    // we then draw the edges of the cuboid on the frame with a cyan color
                    cv::line(frame, projectedCuboid[0], projectedCuboid[1], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[1], projectedCuboid[2], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[2], projectedCuboid[3], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[3], projectedCuboid[0], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[4], projectedCuboid[5], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[5], projectedCuboid[6], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[6], projectedCuboid[7], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[7], projectedCuboid[4], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[0], projectedCuboid[4], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[1], projectedCuboid[5], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[2], projectedCuboid[6], cv::Scalar(255, 255, 0), 2);
                    cv::line(frame, projectedCuboid[3], projectedCuboid[7], cv::Scalar(255, 255, 0), 2);
                    }
                }
                else{
                    // print all other values
                    std::cout << "Failed to solve PnP" << std::endl;
                    // print corners
                    std::cout << "Corners: " << std::endl;
                    for(int i = 0; i < corners.size(); i++){
                        std::cout << corners[i] << std::endl;
                    }
                    // print camera matrix
                    std::cout << "Camera Matrix: " << std::endl;
                    std::cout << cameraMatrix << std::endl;
                    // print distortion
                    std::cout << "Distortion: " << std::endl;
                    std::cout << distorstion << std::endl;
                }
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
                cameraMatrix = cv::Mat::eye(3, 3, CV_64FC1);
                cameraMatrix.at<double>(0, 2) = frame.cols/2;
                cameraMatrix.at<double>(1, 2) = frame.rows/2;
                distorstion = cv::Mat::zeros(5, 1, CV_64F);
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
            if(key == 'e'){
               // extension 
               extension =! extension;
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


