#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include "JPEGCompressor.h"

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
	Mat_<Vec3b> img = imread("images/Lena.bmp", IMREAD_COLOR);

	JPEGCompressor program(img);
	return 0;
}