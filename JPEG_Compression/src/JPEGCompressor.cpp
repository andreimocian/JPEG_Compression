#include "JPEGCompressor.h"

JPEGCompressor::JPEGCompressor(Mat_<Vec3b> initial_img)
{
	Mat_<Vec3b> ycrcb_img;
	this->rows = initial_img.rows;
	this->cols = initial_img.cols;
	this->padded_channels.resize(3);
	cvtColor(initial_img, ycrcb_img, COLOR_BGR2YCrCb);

	split_Y_Cr_Cb(ycrcb_img);
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

void JPEGCompressor::image_padding()
{
	int padded_rows = 0;
	int padded_cols = 0;
	
	for (int i = 0; i < 3; i++)
	{
		padded_rows = this->rows % 8 == 0 ? 0 : (8 - this->rows % 8);
		padded_cols = this->cols % 8 == 0 ? 0 : (8 - this->cols % 8);

		copyMakeBorder(this->channels[i], this->padded_channels[i], 0, padded_rows, 0, padded_cols, BORDER_REPLICATE);
	}
	this->rows_with_padding = this->padded_channels[0].rows;
	this->cols_with_padding = this->padded_channels[0].cols;

	imshow("Y", this->padded_channels[0]);
	imshow("Cr", padded_channels[1]);
	imshow("Cb", padded_channels[2]);
	std::cout << this->rows << " " << this->cols << "\n";
	std::cout << this->rows_with_padding << " " << this->cols_with_padding << "\n";
	waitKey(0);
}

static Mat_<float> convert_to_float(Mat_<uchar> img)
{
	Mat_<float> intermidiate(img.rows, img.cols);
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			intermidiate(i, j) = (float)img(i, j);
		}
	}
	return intermidiate;
}

void JPEGCompressor::process_blocks()
{
	for (int channel = 0; channel < 3; channel++)
	{
		for (int i = 0; i < this->rows_with_padding; i += 8)
		{
			for (int j = 0; j < this->cols_with_padding; j += 8)
			{
				Mat_<uchar> block_8x8 = padded_channels[channel](Rect(j, i, 8, 8));

				Mat_<float> block_8x8_float = convert_to_float(block_8x8);

				block_8x8_float -= 128.0f;


			}
		}
	}
}

void JPEGCompressor::f_dct()
{

}

void JPEGCompressor::compress()
{
	this->image_padding();
	this->process_blocks();
}