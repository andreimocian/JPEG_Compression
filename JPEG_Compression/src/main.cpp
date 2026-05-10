#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/utils/logger.hpp>

using namespace cv;

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
	Mat_<Vec3b> img(256, 256);
	Vec3b white(255, 255, 255);
	Vec3b red(0, 0, 255);
	Vec3b green(0, 255, 0);
	Vec3b yellow(0, 255, 255);

	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			if (i < img.rows / 2 && j < img.cols / 2)
			{
				img(i, j) = white;
			}
			if (i > img.rows / 2 && j < img.cols / 2)
			{
				img(i, j) = red;
			}
			if (i < img.rows / 2 && j > img.cols / 2)
			{
				img(i, j) = green;
			}
			if (i > img.rows / 2 && j > img.cols / 2)
			{
				img(i, j) = yellow;
			}
		}
	}

	imshow("created image", img);
	waitKey();
	return 0;
}