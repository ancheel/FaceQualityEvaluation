#include "stdafx.h"
#include "facepreprocessengine.h"
#include <queue>
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;

typedef struct ConnectedComponentDescription
{
	int ccType; //��ͨ������� ������ͨ��������˵������һ����ͨ��������Ϊ1���ڶ�����ͨ��������Ϊ2.��������
	int ccPixelCount; //��ͨ�������ظ���
	double ccPixelMeanValue; //��ͨ�������ص�ƽ��ֵ
}ConnectedComponentDes;

FacePreProcessEngine::FacePreProcessEngine()
	:backgroundRemovalThreshold(3000), depthSimilarityThreshold(0.3), validFaceAreaRatio(0.8)
{

}

FacePreProcessEngine::FacePreProcessEngine(unsigned int bgThreshold /* = 3000 */, double depthSimThreshold /* = 0.3 */, double ratio /* = 0.8 */)
	: backgroundRemovalThreshold(3000), depthSimilarityThreshold(0.3), validFaceAreaRatio(0.8)
{
	init(bgThreshold, depthSimThreshold, ratio);
}

FacePreProcessEngine::~FacePreProcessEngine()
{

}

void FacePreProcessEngine::init(unsigned int bgThreshold /* = 3000 */, double depthSimThreshold /* = 0.3 */, double ratio /* = 0.8 */)
{
	backgroundRemovalThreshold = bgThreshold;
	depthSimilarityThreshold = depthSimThreshold;
	validFaceAreaRatio = ratio;


	return;
}

void FacePreProcessEngine::proceed(Mat& originDepthImg, vector<Rect>& faceBoxes, cv::Mat& outputDepthImg)
{
	Rect biggestFaceBox;
	Mat  croppedBiggestFace;
	Mat  clonedDepthFace;
	vector<ConnectedComponentDes> ccList;
	vector<vector<Point> > allCcPointsLists;

	findBiggiestFaceBox(faceBoxes, biggestFaceBox);

	croppedBiggestFace = originDepthImg(biggestFaceBox); //�������������������Ϣ�ü�����

	imshow("cropped depth image", croppedBiggestFace);

	clonedDepthFace = croppedBiggestFace.clone();

	normalizeCroppedFace(clonedDepthFace);

	//ccList�洢�����ҵ���������ͨ���������Ϣ��allCcPointsLists�洢������ͨ�������е������
	findAllConnectedComponment(clonedDepthFace, ccList, allCcPointsLists);

	//��ȡ�����ͨ����������ֵ��ֵΪ0

	extractFaceArea(ccList, allCcPointsLists, croppedBiggestFace, outputDepthImg);

	renormalizeDepthFace(outputDepthImg);

	return;
}

unsigned int FacePreProcessEngine::getBgRemovalThreshold()
{
	return backgroundRemovalThreshold;
}

double FacePreProcessEngine::getDepthSimilarityThreshold()
{
	return depthSimilarityThreshold;
}

double FacePreProcessEngine::getValidFaceAreaRatio()
{
	return validFaceAreaRatio;
}

void FacePreProcessEngine::setBgRemovalThreshold(unsigned int threshold)
{
	backgroundRemovalThreshold = threshold;

	return;
}

void FacePreProcessEngine::setDepthSimilarityThreshold(double threshold)
{
	depthSimilarityThreshold = threshold;

	return;
}

void FacePreProcessEngine::setValidFaceAreaRatio(double ratio)
{
	validFaceAreaRatio = ratio;

	return;
}

void FacePreProcessEngine::findBiggiestFaceBox(vector<Rect>& faceBoxes, Rect& biggestFaceBox)
{
	unsigned int areaOfBiggestFaceBox = 0;
	unsigned int areaOfcurrentFaceBox;
	vector<Rect>::iterator iter;

	for (iter = faceBoxes.begin(); iter != faceBoxes.end(); iter++)
	{
		areaOfcurrentFaceBox = (*iter).area();

		if (areaOfcurrentFaceBox > areaOfBiggestFaceBox)
		{
			areaOfBiggestFaceBox = areaOfcurrentFaceBox;
			biggestFaceBox = *iter;
		}
	}

	return;
}

void FacePreProcessEngine::setRemovedPixelValueToZero(Mat& img)
{
	int rowsNum = img.rows;
	int colsNum = img.cols * img.channels();

	for (int i = 0; i < rowsNum; i++)
	{
		float* rowdata = img.ptr<float>(i);

		for (int j = 0; j < colsNum; j++)
		{
			if (rowdata[j] > backgroundRemovalThreshold)
			{
				rowdata[j] = 0;
			}
		}
	}

	return;
}

void FacePreProcessEngine::findMaxAndMinPixelValue(Mat& img, int *p_maxPixelValue, int *p_minPixelValue)
{

	int rowsNum = img.rows;
	int colsNum = img.cols * img.channels();
	int initalized = false;

	for (int i = 0; i < rowsNum; i++)
	{
		float* rowdata = img.ptr<float>(i);

		for (int j = 0; j < colsNum; j++)
		{
			/* ����ͼ�񾭹�setPixelValueBiggerThanThreadholdToZero����󣬲���Ԫ��
			�����ص�ֵΪ0�������轫�ⲿ��Ԫ�غ��ԣ�������Сֵ�϶���0 */
			if (0 == rowdata[j])
			{
				continue;
			}

			if (false == initalized)
			{
				*p_maxPixelValue = rowdata[j];
				*p_minPixelValue = rowdata[j];
				initalized = true;
				continue;
			}

			if (rowdata[j] > *p_maxPixelValue)
			{
				*p_maxPixelValue = rowdata[j];
			}

			if (rowdata[j] < *p_minPixelValue)
			{
				*p_minPixelValue = rowdata[j];
			}
		}
	}

	return;
}

void FacePreProcessEngine::normalizeFaceImg(Mat& img, int maxPixelValue, int minPixelValue, int valSetToPixelValEquZero)
{
	int rowsNum = img.rows;
	int colsNum = img.cols * img.channels();
	int range = maxPixelValue - minPixelValue;

	for (int i = 0; i < rowsNum; i++)
	{

		float* rowdata = img.ptr<float>(i);

		for (int j = 0; j < colsNum; j++)
		{
			//����Ҳ������ֵΪ0�����ص�
			if (0 == rowdata[j])
			{
				//���matlabԴ���е�norm_face(sub2ind(size(norm_face), m, n))=100;�߼�
				rowdata[j] = valSetToPixelValEquZero;
				continue;
			}

			rowdata[j] = (float)(rowdata[j] - minPixelValue) / range;
		}
	}

	return;
}

void FacePreProcessEngine::normalizeCroppedFace(Mat& croppedFace)
{
	int maxPixelValue;
	int minPixelValue;

	//��Mat ת��Ϊ������Mat
	croppedFace.convertTo(croppedFace, CV_32FC1);

	setRemovedPixelValueToZero(croppedFace);

	findMaxAndMinPixelValue(croppedFace, &maxPixelValue, &minPixelValue);

	normalizeFaceImg(croppedFace, maxPixelValue, minPixelValue, 100);
	//���matlabԴ���е�norm_face(sub2ind(size(norm_face), m, n))=100;�߼�

	/*% Set the background value to 1
	[m, n] = find(depth_crop == Inf);
	norm_face(sub2ind(size(norm_face), m, n)) = 100;*/
	/*
	MatlabԴ��������Ҫ��ɵ�������ʵ�ǣ��ҵ�depth_crop������ֵΪ0�ĵ�����꣬Ȼ��norm_face��
	��ͬ�����ϵ�ֵ����Ϊ100��
	*/
	return;
}

bool FacePreProcessEngine::isTwoPointsConnected(double checkedPonitPixelValue, double currentCcTypePixelMeanValue)
{
	double differenceValue = checkedPonitPixelValue - currentCcTypePixelMeanValue;
	double pointDistance;

	pointDistance = sqrt(differenceValue * differenceValue);

	if (pointDistance <= depthSimilarityThreshold)
	{
		return true;
	}

	return false;
}

double FacePreProcessEngine::updateCurrentPixelMeanValue(Mat& inputNormFace, vector<Point> & curentCcPointList)
{
	double totalPixelValue = 0;
	int curentCcSize = curentCcPointList.size();

	for (Point currentPoint : curentCcPointList)
	{
		totalPixelValue += inputNormFace.at<float>(currentPoint);
	}

	return (totalPixelValue / curentCcSize);
}

void FacePreProcessEngine::updateInputNormFace(vector<Point> &curentCc, double currentCcTypePixelMeanValue, Mat& inputNormFace)
{
	for (Point currentPoint : curentCc)
	{
		inputNormFace.at<float>(currentPoint) = currentCcTypePixelMeanValue;
	}

	return;
}


void FacePreProcessEngine::findAllConnectedComponment(Mat& inputNormFace, vector<ConnectedComponentDes>& ccList, vector<vector<Point> >& allCcPointsLists)
{
	/*
	������ͨ��������˵����
	��һ����ͨ��������Ϊ1���ڶ�����ͨ��������Ϊ2.��������

	������ͨ������㷨˵����
	1�������ӵڶ��еڶ��е����ص㿪ʼ
	*/

	int currentCcType = 1;	//����ǰ��ͨ�������ʼ��Ϊ1

	double currentCcTypePixelMeanValue; // = inputNormFace.at<float>(1, 1); //����opencv��mat�ṹ�������±��Ǵ�0��ʼ�ģ��������Ҫ��ȡ�ڶ��еڶ��е�Ԫ�أ�at����������Ϊ1��1

	const vector<Point> directions = { Point(-1,0), Point(0, 1), Point(1, 0), Point(0, -1) }; //�ֱ��ʾ�ϡ��ҡ��¡����ĸ�����
	Mat classMatrix = Mat::zeros(inputNormFace.size(), CV_8UC1);
	//classMatrix.at<uchar>(1, 1) = firstCcClassPixelValue;

	queue<Point> currentPoints;
	Point currentPoint;

	Mat visited = Mat::zeros(inputNormFace.size(), CV_8UC1);
	int faceCols = inputNormFace.cols;
	int faceRows = inputNormFace.rows;

	int faceColsMaxIndex = faceCols - 1;
	int faceRowsMaxIndex = faceRows - 1;

	double currentCcTypeTotalPixelValue;
	unsigned currentCcTypePixelCount;

	//����һ��vector�������浱ǰ��ͨ���е�Ԫ�أ��Ա���¾�ֵ
	vector<Point> curentCcPointsList;
	//set<Point> uniqueCcPointsList;
	ConnectedComponentDes currentCcDes;

	for (int i = 1; i < faceRowsMaxIndex; i++)
	{
		for (int j = 1; j < faceColsMaxIndex; j++)
		{
			if (true == visited.at<uchar>(Point(i, j)))
			{
				continue;
			}

			currentPoints.push(Point(i, j));
			curentCcPointsList.push_back(Point(i, j));
			//uniqueCcPointsList.insert(Point(i, j));

			//��ʼ����ǰ��ֵ
			currentCcTypePixelMeanValue = inputNormFace.at<float>(Point(i, j));
			currentCcTypeTotalPixelValue = currentCcTypePixelMeanValue;
			currentCcTypePixelCount = 1;

			while (true != currentPoints.empty())
			{
				currentPoint = currentPoints.front();
				currentPoints.pop();

				if (true == visited.at<uchar>(currentPoint))
				{
					continue;
				}

				visited.at<uchar>(currentPoint) = true;

				classMatrix.at<uchar>(currentPoint) = currentCcType;

				for (Point dir : directions)
				{
					int x = currentPoint.x + dir.x;//{ Point(-1,0), Point(0, 1), Point(1, 0), Point(0, -1) }; //�ֱ��ʾ�ϡ��ҡ��¡����ĸ�����
					int y = currentPoint.y + dir.y;

					///�ж�x �� y�����Ƿ��ڱ߽���(���߽߱�֮��)��������򲻴���
					if (x < 1 || x > faceRowsMaxIndex - 1 || y < 1 || y > faceColsMaxIndex - 1)
					{
						continue;
					}

					if (true == visited.at<uchar>(Point(x, y)))
					{
						continue;
					}

					//�ж�һ�µ�ǰ���루x��y���Ƿ�����ͨ�ģ����������ͨ�ģ��򲻴���õ�
					double checkedPonitPixelValue = inputNormFace.at<float>(Point(x, y));

					if (true == isTwoPointsConnected(checkedPonitPixelValue, currentCcTypePixelMeanValue))
					{
						currentPoints.push(Point(x, y));

						curentCcPointsList.push_back(Point(x, y));
						//uniqueCcPointsList.insert(Point(x, y));

						currentCcTypeTotalPixelValue += checkedPonitPixelValue;
						currentCcTypePixelCount++;
					}
				}

				currentCcTypePixelMeanValue = currentCcTypeTotalPixelValue / currentCcTypePixelCount;

			}//end of while

			updateInputNormFace(curentCcPointsList, currentCcTypePixelMeanValue, inputNormFace);

			currentCcDes.ccType = currentCcType;
			currentCcDes.ccPixelMeanValue = currentCcTypePixelMeanValue;
			currentCcDes.ccPixelCount = curentCcPointsList.size();
			//currentCcDes.ccPixelCount = uniqueCcPointsList.size();

			ccList.push_back(currentCcDes);

			allCcPointsLists.push_back(curentCcPointsList);

			if (true != curentCcPointsList.empty())
			{
				curentCcPointsList.clear();
			}

			//whileѭ������֮��˵���Ѿ��ҵ���һ����ͨ�����ʱ��͵�����һ����ͨ���ˡ�������Ҫ����ͨ������ͱ�ʶ��1.
			currentCcType++;
		}
	}

	return;
}

bool FacePreProcessEngine::comparePixelValue(const ConnectedComponentDes& first, const ConnectedComponentDes& second)
{
	if (first.ccPixelMeanValue < second.ccPixelMeanValue)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void FacePreProcessEngine::extractFaceArea(vector<ConnectedComponentDes>& ccList, vector<vector<Point> >& allCcPointsLists, Mat& OriginDepthFace, Mat& outputDepthFace)
{
	/*
	1, ��Ҫ֪�������ͨ�����������ص����ꡣ
	2����Ҫ����ÿ����ͨ���ڵ����صĸ�����
	3����Ҫ֪��ÿ����ͨ���ڵĵ�����ֵ����ͨ���ڵ�����ֵ����ȵģ�
	4����Ҫ����ͨ���ڵ�����ֵ��������
	5����Ҫȥ������ֵ�Ƚϴ����Щ��ͨ�򣬲���ʣ����ͨ�����ҳ������ͨ��
	*/

	/* ��ccList ������ͨ���ƽ�����ֵ��С�������� */
	sort(ccList.begin(), ccList.end(), comparePixelValue);

	int ccCount = ccList.size();

	int retainedCcCount = floor(ccCount * validFaceAreaRatio);

	//ɾ������ccList�к���ļ���ֵ
	ccList.erase(ccList.begin() + retainedCcCount, ccList.end());

	//�ҳ�������ظ�������ͨ��
	vector<ConnectedComponentDes>::iterator iter;
	double maxPixelConut = 0;
	ConnectedComponentDes maxPixelValueCc;

	for (iter = ccList.begin(); iter != ccList.end(); iter++)
	{
		if (maxPixelConut < iter->ccPixelCount)
		{
			maxPixelConut = iter->ccPixelCount;
			maxPixelValueCc = *iter;
		}
	}

	vector<Point>& maxPixelValueCcPointList = allCcPointsLists[maxPixelValueCc.ccType - 1];
	vector<Point>::iterator pointsIter;

	//����ȫͼ����ԭ���ͼ���������ͨ���е����ظ�ֵ��outputDepthFace
	for (pointsIter = maxPixelValueCcPointList.begin(); pointsIter != maxPixelValueCcPointList.end(); pointsIter++)
	{
		outputDepthFace.at<ushort>(*pointsIter) = OriginDepthFace.at<ushort>(*pointsIter);
	}

	return;
}

void FacePreProcessEngine::renormalizeDepthFace(Mat& depthFace)
{
	int maxPixelValue;
	int minPixelValue;

	//��Mat ת��Ϊ������Mat
	depthFace.convertTo(depthFace, CV_32FC1);

	findMaxAndMinPixelValue(depthFace, &maxPixelValue, &minPixelValue);

	normalizeFaceImg(depthFace, maxPixelValue, minPixelValue, 1);

	return;
}

unsigned int FacePreProcessEngine::getBiggestCCPixelCountFromdepthFace(Mat& depthface)
{
	//�������ͼ����������������ͨ��
	vector<ConnectedComponentDes> ccList;
	vector<vector<Point> > allCcPointsLists;

	Mat  clonedDepthFace;
	unsigned int BiggestCCPixelCount = 0;
	clonedDepthFace = depthface.clone();
	Mat outputImg;
	outputImg = Mat::zeros(Size(depthface.cols, depthface.rows), depthface.type());
	//�ҵ�������ͨ��
	normalizeCroppedFace(clonedDepthFace);

	findAllConnectedComponment(clonedDepthFace, ccList, allCcPointsLists);

	extractFaceArea(ccList, allCcPointsLists, depthface, outputImg);

	for (int i = 0; i < outputImg.rows; i++)
	{
		for (int j = 0; j < outputImg.cols; j++)
		{
			if (outputImg.at<uchar>(i, j) != 0)
			{
				BiggestCCPixelCount++;
			}
		}
	}
	//BiggestCCPixelCount = getBiggestCCPixelCountFromCC(ccList, allCcPointsLists);

	return BiggestCCPixelCount;
}

unsigned int FacePreProcessEngine::getBiggestCCPixelCountFromCC(vector<ConnectedComponentDes>& ccList, vector<vector<Point> >& allCcPointsLists)
{
	/* ��ccList ������ͨ���ƽ�����ֵ��С�������� */
	sort(ccList.begin(), ccList.end(), comparePixelValue);

	int ccCount = ccList.size();

	int retainedCcCount = floor(ccCount * validFaceAreaRatio);

	//ɾ������ccList�к���ļ���ֵ
	ccList.erase(ccList.begin() + retainedCcCount, ccList.end());

	//�ҳ�������ظ�������ͨ��
	vector<ConnectedComponentDes>::iterator iter;
	double maxPixelConut = 0;
	ConnectedComponentDes maxPixelValueCc;

	for (iter = ccList.begin(); iter != ccList.end(); iter++)
	{
		if (maxPixelConut < iter->ccPixelCount)
		{
			maxPixelConut = iter->ccPixelCount;
			maxPixelValueCc = *iter;
		}
	}

	return maxPixelConut;
}