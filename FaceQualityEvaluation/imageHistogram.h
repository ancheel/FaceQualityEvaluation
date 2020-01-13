#pragma once
#include "util.h"

//����ͼ��ֱ��ͼ
class Histogram1D
{
private:
	int histSize[1];
	float hranges[2];
	const float* ranges[1];
	int channels[1];
public:
	Histogram1D()
	{
		histSize[0] = 256;
		hranges[0] = 0.0;
		hranges[1] = 255.0;//�������ֵ
		ranges[0] = hranges;
		channels[0] = 0;
	}
	cv::Mat getHistogram(const cv::Mat& image)
	{
		cv::Mat hist;
		cv::calcHist(&image,
			1,//��Ϊһ��ͼ���ֱ��ͼ
			channels,//ʹ�õ�ͨ��
			cv::Mat(),//��ʹ������
			hist,//��Ϊ�����ֱ��ͼ
			1,//��ʱһά��ֱ��ͼ
			histSize,//��������
			ranges//����ֵ�ķ�Χ
			);
		return hist;
	}
};