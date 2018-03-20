#pragma once
/* This head file includs the CPFMAccess file for getting access to	*/
#include "stdio.h"
#include "stdlib.h"
// contributed by Mao 
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <string>
#include "math.h"
#include <vector>
// end
using namespace std;
using namespace cv;

class CPFMAccess
{
public:
	// contributed by Dr.Qin web: https://u.osu.edu/qin.324/
	// project1-OSUcivil7453 
	CPFMAccess(void);
	~CPFMAccess(void);

	bool LoadFromFile(char* fn);
	bool SaveToFile(char* fn);
	float* GetData() { return m_data; }
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	bool SetSize(int width, int height);
	bool SetData(float* data);

	void SetPixelValue(int x, int y, const float * value);
	void SetPixelValue(int x, int y, const float r, const float g, const float b);
	void SetPixelBValue(int x, int y, const float b);
	void SetPixelGValue(int x, int y, const float g);
	void SetPixelRValue(int x, int y, const float r);
	void SetPixelComponentValue(int x, int y, const int component_id, const float v);

	void GetPixelValue(int x, int y, float* r, float* g, float* b);
	void GetPixelValue(int x, int y, float *value);
	void GetPixelRValue(int x, int y, float* value);
	void GetPixelGValue(int x, int y, float* value);
	void GetPixelBValue(int x, int y, float* value);
	void GetPixelComponentValue(int x, int y, int component_id, float* value);
	int ReadPFMHead(FILE * pFile, int* width, int* height);

	
public:
	float * m_data;
	int m_width;
	int m_height;



};

// contrinuted by Mao
// email: lm1448892767@outlook.com
Mat GetGrayImage(char* imgpath, int width, int height, float* thresh);
Mat GetRGBImage(char* imgpath);
Mat GetSourcelight(char* lamberpath, char* specpath, char* imgpath, int width, int height, float *Intensity);
Mat RerenderImg(Mat Normalmap, Mat Albedomap, Mat lightsource, Mat objmask, int height, int width);