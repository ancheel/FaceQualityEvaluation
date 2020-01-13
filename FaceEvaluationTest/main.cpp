#include <stdlib.h>
#include <opencv2\opencv.hpp>
#include "FaceQualityEvaluation.h"

using namespace std;
using namespace cv;

/*
���³�����Ҫ��ɶ�FaceQualityEvaluation.dll��̬��Ĳ���
*/
int main()
{
	//��ȡIRͼƬ
	Mat irFrame = cv::imread("./testImages/ir_3.png", CV_LOAD_IMAGE_UNCHANGED);

	cv::Mat ir8BitMat;

	// ����ͼ��10λת8λ
	irFrame.convertTo(ir8BitMat, CV_8UC1, 0.25);

	//ע������㷨��Ҫ8λ��IRͼ���д���

	//��ʼ���㷨ģ��
	std::string facedetect_model_path = std::string("./models/face_detection_models/retina");
	initFaceEvaluation(facedetect_model_path);

	//�������
	cv::Rect2f maxface;

	Anchor anchorFace;
	anchorFace = detectMaxFace(ir8BitMat);

	maxface = anchorFace.finalbox;

	//������
	std::cout << "max face x = " << maxface.x << std::endl;
	std::cout << "max face y = " << maxface.y << std::endl;
	std::cout << "max face width = " << maxface.width << std::endl;
	std::cout << "max face height = " << maxface.height << std::endl;

	//��ȡ���ͼ
	Mat depthFrame = cv::imread("./testImages/depth_3.png", CV_LOAD_IMAGE_UNCHANGED);

	//��ȡ������̬
	FacePosition_s facepose;

	getFacePose(ir8BitMat, depthFrame, anchorFace, &facepose);

	std::cout << "Pitch = " << facepose.Pitch << std::endl;
	std::cout << "Yaw = " << facepose.Yaw << std::endl;
	std::cout << "Roll = " << facepose.Roll << std::endl;

	//��ȡ�����������������ۺ�ģ��������
	FaceQuality faceQuality;
	memset(&faceQuality, 0, sizeof(FaceQuality));
	getFaceQuality(ir8BitMat, depthFrame, anchorFace, &faceQuality);

	cout << "Face illumination Quality = " << faceQuality.illumQuality << endl;
	cout << "Face Blur Quality = " << faceQuality.blurQuality << endl;
	cout << "Face Integrity = " << faceQuality.depthFaceIntegrity << endl;
	cout << "Face faceMacCcRatio = " << faceQuality.depthFaceMaxCcRatio << endl;
	cout << "Face layerCount = " << faceQuality.depthFaceLayerCount << endl;
	cout << "Face precision = " << faceQuality.depthFacePrecision << endl;

	system("pause");
	return 0;
}
