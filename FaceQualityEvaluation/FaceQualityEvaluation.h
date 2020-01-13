#pragma once

#ifdef FACEQUALITYEVALUATION_EXPORTS
#define FACE_EVAL_API __declspec(dllexport)  
#else
#define FACE_EVAL_API __declspec(dllimport)  
#endif

#include <opencv2\opencv.hpp>
#include "anchor.h"

//�ṹ�嶨��
typedef struct {
	float Pitch;
	float Yaw;
	float Roll;
}FacePosition_s;

struct FaceQuality {
	float illumQuality; //����������ȡֵ��Χ0-1��ֵԽСԽ
	float blurQuality;  //ģ����������ȡֵ��Χ0-1��ֵԽСԽ����
	float depthFaceIntegrity; //�������������
	float depthFaceMaxCcRatio; //������������ͨ��ռ��
	int depthFaceLayerCount; //�����������
	float depthFacePrecision; //���������������ֵ
};

//����ӿڶ���
FACE_EVAL_API void initFaceEvaluation(std::string face_dectect_model_path);

FACE_EVAL_API Anchor detectMaxFace(cv::Mat irFrame);

FACE_EVAL_API void getFacePose(cv::Mat irFrame, cv::Mat depthFrame, Anchor face, FacePosition_s *p_facepose);

FACE_EVAL_API void getFaceQuality(cv::Mat irFrame, cv::Mat depthFrame, Anchor face, FaceQuality *p_facequality);

//FACE_EVAL_API float getDeepFaceIntegrity(cv::Mat depthFrame, Anchor face);
//
//FACE_EVAL_API float getDeepFaceMaxCCRatio(cv::Mat depthFrame, Anchor face);
//
//FACE_EVAL_API void getDeepFaceLayerCountAndPrecision(cv::Mat depthFrame, Anchor face, int *pLayerCount, float *pPrecision);