// FaceQualityEvaluation.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "FaceQualityEvaluation.h"
#include <opencv2\opencv.hpp>

int panny(int i, int(*call_back)(int a, int b))
{
	int aa;
	aa = i*i;
	call_back(i, aa);
	return 0;
}

Anchor detectMaxFace(cv::Mat irFrame)
{
	Anchor maxFace;

	return maxFace;
}