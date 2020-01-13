// FaceQualityEvaluation.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "FaceQualityEvaluation.h"
#include <opencv2\opencv.hpp>
#include <opencv2\core\fast_math.hpp>
#include <vector>
#include "tools.h"
#include "face_pose_three_degree.h"
#include "imageProcess.h"
#include "illuminationDetect.h"
#include "blurDetect.h"
#include "facepreprocessengine.h"

using namespace cv;
using namespace std;

//�궨����
#define MAX_DEPTH_VALUE 10000

//ȫ�����ݶ�����
ncnn::Net g_facedectec_net;
FacePoseEstimate g_face_pose;

/*
�ڲ�����������
*/
extern float getDeepFaceIntegrity(cv::Mat depthFrame, Anchor face);
extern float getDeepFaceMaxCCRatio(cv::Mat depthFrame, Anchor face);
extern void getDeepFaceLayerCountAndPrecision(cv::Mat depthFrame, Anchor face, int *pLayerCount, float *pPrecision);

int find_max_face_retinaFace(vector<Anchor> &result, Anchor &maxFace) {
	int max_index = 0;
	float max_area = 0.0;
	for (int i = 0; i < result.size(); ++i) {
		result[i].finalbox.width = result[i].finalbox.width - result[i].finalbox.x;
		result[i].finalbox.height = result[i].finalbox.height - result[i].finalbox.y;
		if (max_area < result[i].finalbox.area()) {
			max_area = result[i].finalbox.area();
			max_index = i;
		}
	}
	maxFace = result[max_index];
	return 0;
}


int load_modal(ncnn::Net &net, std::string model_path = "./models/retina") {
	std::string param_path = model_path + ".param";
	std::string bin_path = model_path + ".bin";
	net.load_param(param_path.data());
	net.load_model(bin_path.data());
	return 0;
}

int model_forward(ncnn::Mat &input, ncnn::Net &_net, std::vector<Anchor> &proposals) {
	input.substract_mean_normalize(pixel_mean, pixel_std);
	ncnn::Extractor _extractor = _net.create_extractor();
	_extractor.input("data", input);

	proposals.clear();

	vector<AnchorGenerator> ac(_feat_stride_fpn.size());
	for (int i = 0; i < _feat_stride_fpn.size(); ++i) {
		int stride = _feat_stride_fpn[i];
		ac[i].Init(stride, anchor_cfg[stride], false);
	}

	for (int i = 0; i < _feat_stride_fpn.size(); ++i) {
		ncnn::Mat cls;
		ncnn::Mat reg;
		ncnn::Mat pts;

		// get blob output
		char clsname[100]; sprintf_s(clsname, 100, "face_rpn_cls_prob_reshape_stride%d", _feat_stride_fpn[i]);
		char regname[100]; sprintf_s(regname, 100, "face_rpn_bbox_pred_stride%d", _feat_stride_fpn[i]);
		char ptsname[100]; sprintf_s(ptsname, 100, "face_rpn_landmark_pred_stride%d", _feat_stride_fpn[i]);
		_extractor.extract(clsname, cls);
		_extractor.extract(regname, reg);
		_extractor.extract(ptsname, pts);

		ac[i].FilterAnchor(cls, reg, pts, proposals);
	}
	return 0;
}

/*
����Ϊ��������������
*/

/*
�ڳ�ʼ��ģ���м���nnģ��
*/
void initFaceEvaluation(std::string face_dectect_model_path)
{
	//�����������ģ��
	//std::string model_path = "../models/face_detection_models/retina";
	load_modal(g_facedectec_net, face_dectect_model_path);

	// ���������ڵ��ж�ģ��
	//PCANet pcaNet;
	//PCA_Train_Result* result = new PCA_Train_Result;
	//std::string svmpath = "../../../getFaceQuality/face_occlusion_model/svm.xml";
	//cv::Ptr<cv::ml::SVM> SVM;
	//SVM = cv::ml::StatModel::load<cv::ml::SVM>(svmpath);
	//getPCANetPara(result, pcaNet);
}

Anchor detectMaxFace(cv::Mat irFrame)
{
	cv::Rect2f maxFaceRect;
	Anchor maxFace;


	// ����ͼ��ͨ��ת��ͨ��
	Mat irMatc3[3], irDivideMat3;
	irMatc3[0] = irFrame.clone();
	irMatc3[1] = irFrame.clone();
	irMatc3[2] = irFrame.clone();
	cv::merge(irMatc3, 3, irDivideMat3);

	// ��߶��������
	ncnn::Mat input = ncnn::Mat::from_pixels_resize(irDivideMat3.data, ncnn::Mat::PIXEL_BGR2RGB, irDivideMat3.cols, irDivideMat3.rows, 640, 400);
	std::vector<Anchor> proposals;
	model_forward(input, g_facedectec_net, proposals);

	// NMS�Ǽ���ֵ����
	std::vector<Anchor> NmsFacePara;
	nms_cpu(proposals, nms_threshold, NmsFacePara);

	if (NmsFacePara.size() != 0) {

		find_max_face_retinaFace(NmsFacePara, maxFace);
	}

	//maxFaceRect = maxFace.finalbox;

	return maxFace;
}


void getFacePose(cv::Mat irFrame, cv::Mat depthFrame, Anchor face, FacePosition_s *p_facepose)
{
	// ����ͼ��ͨ��ת��ͨ��
	Mat irMatc3[3], irDivideMat3;
	irMatc3[0] = irFrame.clone();
	irMatc3[1] = irFrame.clone();
	irMatc3[2] = irFrame.clone();
	cv::merge(irMatc3, 3, irDivideMat3);

	// ������̬����
	g_face_pose.updata(face, depthFrame, irDivideMat3);
	g_face_pose.calFacePose();
	float *pose = g_face_pose.getAngle();

	float Pitch = pose[0];
	float Yaw = pose[1];
	float Roll = pose[2];

	p_facepose->Pitch = Pitch;
	p_facepose->Yaw = Yaw;
	p_facepose->Roll = Roll;

	//����һ���ṹ��ͬʱ����������ֵ
	return;
}

//��ȡIRͼ�������

//��ȡIRͼ���ģ����

//��ԭʼ��ʵ���У������Ⱥ�ģ������Щָ��ļ��㶼�ۺϵ�������һ��������
void getFaceQuality(cv::Mat irFrame, cv::Mat imagedepth, Anchor face, FaceQuality *pfaceQuality)
{
	float blur, illumination;
	cv::Mat imgresize, imageprs, image_cut, imageprs_cut;

	cv::Mat image = irFrame(face.finalbox);

	double scale = 0.25;
	cv::Size dsize = cv::Size(image.cols*scale, image.rows*scale);
	cv::resize(image, imgresize, dsize, 0, 0, 1);

	//��������������ؼ��������ת�������������������
	for (int i = 0; i < 5; i++)
	{
		face.pts[i].x -= face.finalbox.x;
		face.pts[i].y -= face.finalbox.y;
	}

	imageProcess(image, face.pts, imageprs, image_cut, imageprs_cut);
	illuminationDetect(image, imageprs_cut, illumination);
	float illumScale = (illumination - minILLMEAN) / (maxILLMEAN - minILLMEAN);
	pfaceQuality->illumQuality = illumScale;
	if (illumScale < 0)
	{
		pfaceQuality->illumQuality = 1;
		return;
	}

	float distance = 0;
	getCamDistance(imagedepth, face.pts, distance);
	blurDetect(distance, illumination, blur);
	pfaceQuality->blurQuality = blur;

	//��ȡ�������������
	pfaceQuality->depthFaceIntegrity = getDeepFaceIntegrity(imagedepth, face);

	//��ȡ���������ͨ��ռ��
	pfaceQuality->depthFaceMaxCcRatio = getDeepFaceMaxCCRatio(imagedepth, face);

	//��ȡ�������ͼ��������������ֵ
	int layerCount = 0;
	float precision = 0;

	getDeepFaceLayerCountAndPrecision(imagedepth, face, &layerCount, &precision);

	pfaceQuality->depthFaceLayerCount = layerCount;
	pfaceQuality->depthFacePrecision = precision;

	return;
}

/*
������������Եļ���
���㹫ʽ��
���������0���ص�ĸ���/���������ܵ����ص�ĸ���
����ֵȡֵ��Χ��0--1
*/
float getDeepFaceIntegrity(cv::Mat depthFrame, Anchor face)
{
	unsigned int deepFacePixelCount = 0;
	unsigned int deepFaceNoneZeroPixelCount = 0;

	cv::Mat deepFace = depthFrame(face.finalbox);

	deepFacePixelCount = deepFace.cols * deepFace.rows;

	for (int i = 0; i < deepFace.rows; i++)
	{
		for (int j = 0; j < deepFace.cols; j++)
		{
			if (deepFace.at<ushort>(i, j) != 0)
			{
				deepFaceNoneZeroPixelCount++;
			}
		}
	}

	return (deepFaceNoneZeroPixelCount * 1.0) / deepFacePixelCount;
}

/*
���º���������ȡ��ͨ��
���㹫ʽ��
�����ͨ���е����ص����/���������ܵ����ص����
����ֵȡֵ��Χ��0--1
*/
float getDeepFaceMaxCCRatio(cv::Mat depthFrame, Anchor face)
{
	unsigned int maxccpixelcount = 0;
	unsigned int deepFacePixelCount = 0;

	float ratio = 0;
	//�������������������Ϣ�ü�����
	Mat  deepFace = depthFrame(face.finalbox);

	//�������ͨ���������Ϣ�У���ȡ�����ͨ�������ص�ĸ���
	FacePreProcessEngine *pfacePreProcessEngine = new FacePreProcessEngine();
	maxccpixelcount = pfacePreProcessEngine->getBiggestCCPixelCountFromdepthFace(deepFace);

	//���ݹ�ʽ�������������ͨ��ռ��
	deepFacePixelCount = deepFace.cols * deepFace.rows;

	ratio = maxccpixelcount*1.0 / deepFacePixelCount;

	delete pfacePreProcessEngine;

	return ratio;

}


int getLayerCountAndMaxMinPixelValue(Mat& deepFace, int *p_maxPixelValue, int *p_minPixelValue)
{
	int initalized = false;

	//ͳ�Ʋ�ͬ���ֵ�ĸ���
	bool *phistogramArray = new bool[MAX_DEPTH_VALUE];
	int layerCount = 0;

	//��ʼ�����ֵ�Ƿ���ֵı��
	for (int i = 0; i < MAX_DEPTH_VALUE; i++)
	{
		phistogramArray[i] = false;
	}


	for (int i = 0; i < deepFace.rows; i++)
	{
		for (int j = 0; j < deepFace.cols; j++)
		{
			int pixelValue = deepFace.at<ushort>(i, j);

			if (pixelValue  == 0)
			{
				continue;
			}

			phistogramArray[pixelValue] = true;

			if (false == initalized)
			{
				*p_maxPixelValue = pixelValue;
				*p_minPixelValue = pixelValue;
				initalized = true;
				continue;
			}

			if (pixelValue > *p_maxPixelValue)
			{
				*p_maxPixelValue = pixelValue;
			}

			if (pixelValue < *p_minPixelValue)
			{
				*p_minPixelValue = pixelValue;
			}
		}
	}

	//������ȵĲ���
	for (int i = 0; i < MAX_DEPTH_VALUE; i++)
	{
		if (true == phistogramArray[i])
		{
			layerCount++;
		}
	}

	delete phistogramArray;

	return layerCount;
}

void getDeepFaceLayerCountAndPrecision(cv::Mat depthFrame, Anchor face, int *pLayerCount, float *pPrecision)
{
	int layerCount = 0;
	int maxPixelValue;
	int minPixelValue;
	Mat  deepFace = depthFrame(face.finalbox);

	layerCount = getLayerCountAndMaxMinPixelValue(deepFace, &maxPixelValue, &minPixelValue);

	*pLayerCount = layerCount;

	*pPrecision = (maxPixelValue - minPixelValue) * 1.0 / (layerCount - 1);

	return;
}