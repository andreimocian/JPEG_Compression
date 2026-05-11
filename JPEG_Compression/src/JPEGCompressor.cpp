#include "JPEGCompressor.h"

JPEGCompressor::JPEGCompressor(Mat_<Vec3b> initial_img)
{
	Mat_<Vec3b> ycrcb_img;
	cvtColor(initial_img, ycrcb_img, COLOR_BGR2YCrCb);

	split_Y_Cr_Cb(ycrcb_img);

	imshow("Y", this->channels[0]);
	imshow("Cr", channels[1]);
	imshow("Cb", channels[2]);
	waitKey(0);
}

void JPEGCompressor::split_Y_Cr_Cb(Mat_<Vec3b> initial_img)
{
	Mat_<uchar> Y(initial_img.rows, initial_img.cols), Cr(initial_img.rows, initial_img.cols), Cb(initial_img.rows, initial_img.cols);
	Vec3b pixel;

	for (int i = 0; i < initial_img.rows; i++)
	{
		for (int j = 0; j < initial_img.cols; j++)
		{
			pixel = initial_img(i, j);
			Y(i, j) = pixel[0];
			Cr(i, j) = pixel[1];
			Cb(i, j) = pixel[2];
		}
	}

	this->channels = { Y, Cr, Cb };
}