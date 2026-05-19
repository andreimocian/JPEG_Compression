#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>
#include "JPEGCompressor.h"

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
	Mat_<Vec3b> img = imread("images/airplane.bmp", IMREAD_COLOR);


	JPEGCompressor compressor(img);
	compressor.compress();
	Mat_<Vec3b> decompressed_img = compressor.decompress("test");


	imshow("Initial image", img);
	imshow("Decompressed image", decompressed_img);
	waitKey(0);
	return 0;
}