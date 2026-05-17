#include "JPEGCompressor.h"

JPEGCompressor::JPEGCompressor(Mat_<Vec3b> initial_img)
{
	Mat_<Vec3b> ycrcb_img;
	this->rows = initial_img.rows;
	this->cols = initial_img.cols;
	this->padded_channels.resize(3);
	cvtColor(initial_img, ycrcb_img, COLOR_BGR2YCrCb);

	split_Y_Cr_Cb(ycrcb_img);

	this->quantization_table_luminance = {
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	};

	this->quantization_table_chrominance = {
		{17, 18, 24, 47, 99, 99, 99, 99},
		{18, 21, 26, 66, 99, 99, 99, 99},
		{24, 26, 56, 99, 99, 99, 99, 99},
		{47, 66, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99}
	};
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

Mat_<float> JPEGCompressor::f_dct(const Mat_<float>& block_8x8_float)
{
	Mat_<float> res(8, 8);
	for (int u = 0; u < 8; u++)
	{
		for (int v = 0; v < 8; v++)
		{
			float Cu = (u == 0 ? 1 / sqrt(2) : 1);
			float Cv = (v == 0 ? 1 / sqrt(2) : 1);

			float sum = 0.0f;
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					sum += block_8x8_float(x, y) * cos(((2 * x + 1) * u * CV_PI) / 16) * cos(((2 * y + 1) * v * CV_PI) / 16);
				}
			}
			res(u, v) = (1.0f / 4.0f) * Cu * Cv * sum;
		}
	}

	return res;
}

Mat_<int> JPEGCompressor::quantization(const Mat_<float>& dct_res, int channel)
{
	Mat_<int> res(8, 8);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (channel == Y_CHANNEL)
				res(i, j) = (int)std::round(dct_res(i, j) / this->quantization_table_chrominance[i][j]);
			else
				res(i, j) = (int)std::round(dct_res(i, j) / this->quantization_table_luminance[i][j]);
		}
	}
	return res;
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

				Mat_<float> dct_res = f_dct(block_8x8_float);

				Mat_<int> quantized_block = quantization(dct_res, channel);
			}
		}
	}
}

void JPEGCompressor::compress()
{
	this->image_padding();
	this->process_blocks();
}