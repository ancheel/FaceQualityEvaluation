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

	//��ʼ���㷨ģ��
	std::string facedetect_model_path = std::string("./models/face_detection_models/retina");
	initFaceEvaluation(facedetect_model_path);

	//�������
	cv::Rect2f maxface;

	Anchor anchorFace;
	anchorFace = detectMaxFace(irFrame);

	maxface = anchorFace.finalbox;

	//������
	std::cout << "max face x = " << maxface.x << std::endl;
	std::cout << "max face y = " << maxface.y << std::endl;
	std::cout << "max face width = " << maxface.width << std::endl;
	std::cout << "max face height = " << maxface.height << std::endl;

	system("pause");
	return 0;
}
