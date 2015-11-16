#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "Edge.h"
#include "quicksort.h"
#include "disjoint_set.h"

using namespace cv;
using namespace std;

//read a color image, split it into 3 channels blue, green, red of type double
//reading parameters
bool readInput(Mat &colorImg, vector<Mat> &bgr, double &sigma, double &k) {
	string filepath = "huu.jpg";
	/*cout << "Input filepath of a color image: ";
	cin >> filepath;
	cout << "Input sigma value (applied to Gaussian blurring): ";
	cin >> sigma;
	cout << "Input K: ";
	cin >> k;*/
	colorImg = imread(filepath);
	if (colorImg.empty()) {
		cout << "Reading Image Failed" << endl;
		return false;
	}
	Mat channels[3];
	split(colorImg, channels);
	Mat bl;
	Mat gr;
	Mat re;
	channels[0].convertTo(bl, CV_64F);
	channels[1].convertTo(gr, CV_64F);
	channels[2].convertTo(re, CV_64F);
	bgr.push_back(bl);
	bgr.push_back(gr);
	bgr.push_back(re);
	return true;
}

// apply gaussian blurring to 3 channels of image with sigma value inputted 
void smoothImage(vector<Mat> &bgr, double sigma) {
	GaussianBlur(bgr[0], bgr[0], Size(5, 5), sigma, sigma, BORDER_DEFAULT);
	GaussianBlur(bgr[1], bgr[1], Size(5, 5), sigma, sigma, BORDER_DEFAULT);
	GaussianBlur(bgr[2], bgr[2], Size(5, 5), sigma , sigma, BORDER_DEFAULT);
	return;
}

// weight an edge
double weightEdge(vector<Mat> &bgr, int u1, int v1, int u2, int v2) {
	double db = pow(bgr[0].at<double>(u1, v1) - bgr[0].at<double>(u2, v2), 2.0);
	double dg = pow(bgr[1].at<double>(u1, v1) - bgr[1].at<double>(u2, v2), 2.0);
	double dr = pow(bgr[2].at<double>(u1, v1) - bgr[2].at<double>(u2, v2), 2.0);
	double diff = sqrt(db + dg + dr);
	return diff;
}

//build graph of pixels
void buildGraph(vector<Mat> &bgr, Edge *edges, int &edgeCount) {
	int rows = bgr[0].rows;
	int cols = bgr[0].cols;	
	Edge edge;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {			
			edge.p1 = i*cols + j;

			if (i - 1 >= 0 && j + 1 < cols) {
				edge.p2 = (i - 1)*cols + (j + 1);
				edge.w = weightEdge(bgr, i, j, i - 1, j + 1);
				edges[edgeCount] = edge;
				edgeCount++;
			}			

			if (j + 1 < cols) {
				edge.p2 = i*cols + (j + 1);
				edge.w = weightEdge(bgr, i, j, i, j + 1);
				edges[edgeCount] = edge;
				edgeCount++;
			}			

			if (i + 1 < rows && j + 1 < cols) {
				edge.p2 = (i + 1)*cols + (j + 1);
				weightEdge(bgr, i, j, i + 1, j + 1);
				edges[edgeCount] = edge;
				edgeCount++;
			}			

			if (i + 1 < rows) {
				edge.p2 = (i + 1)*cols + j;
				weightEdge(bgr, i, j, i + 1, j);
				edges[edgeCount] = edge;
				edgeCount++;
			}			
		}
	}
	return;
}

// segment a graph, return the forest of regions 
void segmentGraph(Edge *edges, int vertexCount, int edgeCount, double k, Forest &forest) {	
	double* threshold;
	threshold = new double[vertexCount];
	for (int i = 0; i < vertexCount; i++) {
		threshold[i] = k;
	}
	for (int i = 0; i < edgeCount; i++) {
		int r1 = forest.findRoot(edges[i].p1);
		int r2 = forest.findRoot(edges[i].p2);
		if (r1 != r2) {
			if (edges[i].w <= threshold[r1] && edges[i].w <= threshold[r2]) {
				forest.uniteRoots(r1, r2);
				int r = forest.findRoot(r1);
				threshold[r] = edges[i].w + k / ((double)(forest.getSizeFromRoot(r)));
			}			
		}
	}
	delete[] threshold;
	return;
}

//get segmentation image from segmentation of the graph of pixels
void getSegmentationImage(Forest &forest, Mat &img) {
	int rows = img.rows;
	int cols = img.cols;
	srand(time(NULL));
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			
			uchar b = (uchar)(rand() % 256);
			uchar g = (uchar)(rand() % 256);
			uchar r = (uchar)(rand() % 256);
			img.at<Vec3b>(i, j)[0] = b;
			img.at<Vec3b>(i, j)[1] = g;
			img.at<Vec3b>(i, j)[2] = r;
		}
	}
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int r = forest.findRoot(i*cols + j);
			int i1 = r / cols;
			int j1 = r % cols;
			img.at<Vec3b>(i, j)[0] = img.at<Vec3b>(i1, j1)[0];
			img.at<Vec3b>(i, j)[1] = img.at<Vec3b>(i1, j1)[1];
			img.at<Vec3b>(i, j)[2] = img.at<Vec3b>(i1, j1)[2];
		}
	}
	return;
}

int main() {
	Mat colorImg;
	vector<Mat> bgr;
	double sigma = 0.8;
	double k = 100;
	if (!readInput(colorImg, bgr, sigma, k)) {
		return -1;
	}

	//smooth the image before computing the edge weights in order to compensate for digitization artifacts
	smoothImage(bgr, sigma);

	//build graph
	int edgeCount = 0;
	int vertexCount = bgr[0].rows * bgr[0].cols;
	Edge *edges = new Edge[vertexCount * 4];	
	buildGraph(bgr, edges, edgeCount);

	//sort edges of graph by weight in the ascending order..... using quicksort
	quickSort(edges, 0, edgeCount - 1);	

	//segment the graph of pixels
	Forest forest(vertexCount);
	segmentGraph(edges, vertexCount, edgeCount, k, forest);

	//get output image of sementation
	Mat segmentation(bgr[0].rows, bgr[0].cols, CV_8UC3);
	getSegmentationImage(forest, segmentation);

	//show input and output segmentation images
	imshow("original image", colorImg);
	imshow("segmentation", segmentation);
	waitKey(0);

	return 0;
}