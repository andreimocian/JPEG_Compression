#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

class JPEGCompressor
{
public:
	JPEGCompressor(Mat_<Vec3b> initial_img);

private:
	void split_Y_Cr_Cb(Mat_<Vec3b> initial_img);

	std::vector<Mat_<uchar>> channels;
};

