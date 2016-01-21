#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define BACKGROND_COLOR_CHANNELS 3
#define BACKGROND_COLOR_RED 0
#define BACKGROND_COLOR_GREEN 1
#define BACKGROND_COLOR_BLUE 2

#define MAX_FILE_PATH 255
#define MAX_CLUSTERS 10 /* number of cluster */

using namespace cv;
using namespace std;

void rgb2hsb(int rgbR, int rgbG, int rgbB, float* h, float* s, float* v) {
	int max, min;
	max = (rgbR > rgbG) ? rgbR : rgbG;
	max = (max > rgbB) ? max : rgbB;
	min = (rgbR < rgbG) ? rgbR : rgbG;
	min = (min < rgbB) ? min : rgbB;

	float hsbB = max / 255.0f;
	float hsbS = max == 0 ? 0 : (max - min) / (float)max;

	float hsbH = 0;
	if (max == min)
		;
	else if (max == rgbR && rgbG >= rgbB) {
		hsbH = (rgbG - rgbB) * 60.0 / (max - min) + 0;
	}
	else if (max == rgbR && rgbG < rgbB) {
		hsbH = (rgbG - rgbB) * 60.0 / (max - min) + 360;
	}
	else if (max == rgbG) {
		hsbH = (rgbB - rgbR) * 60.0 / (max - min) + 120;
	}
	else if (max == rgbB) {
		hsbH = (rgbR - rgbG) * 60.0 / (max - min) + 240;
	}

	*h = hsbH;
	*s = hsbS;
	*v = hsbB;
}

int main()
{
	// testing Opencv cConfiguration
	/*Mat img = imread("test.jpg");
	namedWindow("test");
	imshow("test", img);
	waitKey(6000);*/

	int i, j, size, size_mini, K;
	IplImage *src_img = 0, *src_mini_img = 0, *dst_img = 0, *dst_hsv_img = 0;
	CvMat tmp_header;
	CvMat *labels_rgb, *labels_hsv, *points_rgb, *points_hsv, *tmp;
	const char *imagename;
	//set background color
	int background_color[BACKGROND_COLOR_CHANNELS] = { 0,0,0 }; // 0,0,0 fro black color

	//set place u save image file.
	char input[MAX_FILE_PATH];
	char file_name[MAX_FILE_PATH];
	char minifile_name[MAX_FILE_PATH];

	printf("Please input the bmp picture name:\n");
	cin >> input;
	sprintf_s(minifile_name, "%s_mini.bmp", input);
	sprintf_s(file_name, "%s.bmp", input);

	src_img = cvLoadImage(file_name, CV_LOAD_IMAGE_COLOR);
	src_mini_img = cvLoadImage(minifile_name, CV_LOAD_IMAGE_COLOR);
	if (src_img == 0 || src_mini_img == 0)
	{
		printf("ERROR:Cannot open the img file!\n");
		return -1;
	}

	size = src_img->width * src_img->height;
	size_mini = src_mini_img->width * src_mini_img->height;
	labels_rgb = cvCreateMat(size, 1, CV_32SC1);
	labels_hsv = cvCreateMat(size, 1, CV_32SC1);
	points_rgb = cvCreateMat(size, 1, CV_32FC3);
	points_hsv = cvCreateMat(size, 1, CV_32FC3);

	// testing RGB save mode
	/*for (j = 0; j < 3; j++)
	{
		char file[MAX_FILE_PATH] = ""; //generate saved file name.
		sprintf_s(file, "%s%d%s", "RGB", j, ".bmp");
		dst_img = cvCloneImage(src_img);
		for (i = 0; i < size; i++)
		{
			if (j == 0)
			{
				dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
				dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
			}
			else if (j == 1)
			{
				dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
				dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
			}
			else if (j == 2)
			{
				dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
				dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
			}
		}
		cvSaveImage(file, dst_img, 0);
		cvReleaseImage(&dst_img);
	}*/

	// get hsv img from rgb img
	dst_hsv_img = cvCloneImage(src_img);
	cvCvtColor(dst_hsv_img, dst_hsv_img, CV_BGR2HSV);
	//sprintf_s(file_name, "%s_hsv.bmp", input);
	//cvSaveImage(file_name, dst_hsv_img, 0);
	// output the imgdata to the file img_rgb.dat
	// transfer for RGB to HSB and save to the file img_hsb.dat in the same time
	ofstream fout1("img_mini_rgb.dat"), fout2("img_mini_hsb.dat");
	if (fout1.is_open() && fout2.is_open())
	{
		int rgbG, rgbB, rgbR;
		float h, s, v;
		for (i = 0; i < size_mini; i++)
		{
			rgbB = (int)src_mini_img->imageData[i * 3 + 0] + 128;
			rgbG = (int)src_mini_img->imageData[i * 3 + 1] + 128;
			rgbR = (int)src_mini_img->imageData[i * 3 + 2] + 128;
			fout1 << rgbG << ',' << rgbB << ',' << rgbR << endl;
			rgb2hsb(rgbG, rgbB, rgbR, &h, &s, &v);
			fout2 << (int)h << ',' << (int)(s * 100) << ',' << (int)(v * 100) << endl;
		}
		fout1.close();
		fout2.close();
	}

	// format the data to fix the cvKMeans2 function
	// int cvKMeans2(const CvArr* samples, int cluster_count, CvArr* labels, CvTermCriteria termcrit, int attempts=1, CvRNG* rng=0, int flags=0, CvArr* _centers=0, double* compactness=0 )
	// samples ¨C Floating-point matrix of input samples, one row per sample.
	for (i = 0; i < size; i++) {
		points_rgb->data.fl[i * 3 + 0] = (uchar)src_img->imageData[i * 3 + 0];
		points_rgb->data.fl[i * 3 + 1] = (uchar)src_img->imageData[i * 3 + 1];
		points_rgb->data.fl[i * 3 + 2] = (uchar)src_img->imageData[i * 3 + 2];
		points_hsv->data.fl[i * 3 + 0] = (uchar)dst_hsv_img->imageData[i * 3 + 0];
		points_hsv->data.fl[i * 3 + 1] = (uchar)dst_hsv_img->imageData[i * 3 + 1];
		points_hsv->data.fl[i * 3 + 2] = (uchar)dst_hsv_img->imageData[i * 3 + 2];
	}

	// use matlab to find K value with img.dat
	printf("Please input K of clusters:\n");
	cin >> K;
	if (K > MAX_CLUSTERS)
		K = MAX_CLUSTERS;
	CvMat *centers_rgb = cvCreateMat(K, 3, CV_32FC1);
	CvMat *centers_hsv = cvCreateMat(K, 3, CV_32FC1);

	// flags ¨C KMEANS_PP_CENTERS Use kmeans++ center initialization by Arthur and Vassilvitskii [Arthur2007].
	cvKMeans2(points_rgb, K, labels_rgb,
		cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
		1, 0, KMEANS_PP_CENTERS, centers_rgb, 0);

	for (j = 0; j < K; j++)
	{
		char file[MAX_FILE_PATH] = ""; //generate saved file name.
		sprintf_s(file, "%s%d%s", "SegmentationByRGB", j, ".bmp");
		dst_img = cvCloneImage(src_img);
		for (i = 0; i < size; i++)
		{
			int idx = labels_rgb->data.i[i];
			if (j != idx)	// paint the points not belonging to this cluster to black
			{
				dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
				dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
				dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
			}
		}
		cvSaveImage(file, dst_img, 0);
		cvReleaseImage(&dst_img);
		printf("cluster %d image save completed.\n", j);
	}

	// Segmentation by HSV value
	cvKMeans2(points_hsv, K, labels_hsv,
		cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
		1, 0, KMEANS_PP_CENTERS, centers_hsv, 0);

	for (j = 0; j < K; j++)
	{
		char file[MAX_FILE_PATH] = ""; //generate saved file name.
		sprintf_s(file, "%s%d%s", "SegmentationByHSV", j, ".bmp");
		dst_img = cvCloneImage(src_img);
		for (i = 0; i < size; i++)
		{
			int idx = labels_hsv->data.i[i];
			if (j != idx)	// paint the points not belonging to this cluster to black
			{
				dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
				dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
				dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
			}
		}
		cvSaveImage(file, dst_img, 0);
		cvReleaseImage(&dst_img);
		printf("cluster %d image save completed.\n", j);
	}

	cvReleaseImage(&src_img);
	cvReleaseImage(&src_mini_img);
	cvReleaseImage(&dst_img);
	cvReleaseImage(&dst_hsv_img);
	cvReleaseMat(&labels_rgb);
	cvReleaseMat(&labels_hsv);
	cvReleaseMat(&points_rgb);
	return 0;
}
