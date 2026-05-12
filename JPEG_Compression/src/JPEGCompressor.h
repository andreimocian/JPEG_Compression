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
	void compress();

private:
	void split_Y_Cr_Cb(Mat_<Vec3b> initial_img);
	void image_padding();
	void process_blocks();
	void f_dct();

	std::vector<Mat_<uchar>> channels;
	std::vector<Mat_<uchar>> padded_channels;
	int rows, rows_with_padding;
	int cols, cols_with_padding;
};

