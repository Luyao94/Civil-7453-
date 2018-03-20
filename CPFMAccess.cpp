//#include "StdAfx.h"
#include "CPFMAccess.h"
#include "string.h"


CPFMAccess::CPFMAccess(void)
{
	m_data = NULL;
	m_width = -1;
	m_height = -1;
}

CPFMAccess::~CPFMAccess(void)
{
	if (m_data != NULL)
		delete[]m_data;
}


bool CPFMAccess::LoadFromFile(char* fn)
{
	FILE * fp = fopen(fn, "rb");
	if (fp == NULL)
		return false;
	int dataStartPos = ReadPFMHead(fp, &m_width, &m_height);
	if (dataStartPos == 0)
	{
		fclose(fp);
		return false;
	}
	fseek(fp, dataStartPos, SEEK_SET);

	if (m_data != NULL)
	{
		delete[]m_data;
		m_data = NULL;
	}
	m_data = new float[3 * m_width*m_height];
	if (fread(m_data, sizeof(float), 3 * m_width*m_height, fp) != 3 * m_width*m_height)
	{
		delete[]m_data;
		m_data = NULL;
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

bool CPFMAccess::SaveToFile(char* fn)
{
	if (m_data == NULL)
		return false;

	FILE * fp = fopen(fn, "wb");
	if (fp == NULL)
		return false;

	//fprintf(fp ,"PF\x0a%d %d\x0a-1.000000\x0a",m_width, m_height);
	fprintf(fp, "PF\x0a%d\x0a%d\x0a-1.000000\x0a", m_width, m_height); // by zlzhou

	if (fwrite(m_data, sizeof(float), 3 * m_width*m_height, fp) != 3 * m_width*m_height)
	{
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}


int	CPFMAccess::ReadPFMHead(FILE * pFile, int* width, int* height)
{
	char Header[513];
	fseek(pFile, 0, SEEK_SET);
	int	len = fread(Header, sizeof(char), 512, pFile);
	Header[len] = 0;

	if (len > 3)
	{
		Header[len - 1] = 0;
		if ((Header[0] == 'P' && Header[1] == 'F') ||
			(Header[0] == 'p' && Header[1] == 'f'))
		{
			char* p = strchr(Header, 0xa);
			if (p)
			{

				p++;


				//for the read of pfm file generated from photoshop
				int cx, cy;
				char* end;
				end = strchr(p, 0xa);
				end = &(end[1]);
				end = strchr(end, 0xa);
				end[0] = 0;
				if (sscanf(p, "%d %d", &cx, &cy) == 2)
				{
					*width = cx;
					*height = cy;
					p = &end[1];
					end = strchr(p, 0xa);
					if (end)
					{
						return (end - Header) + 1;
					}

				}


				//for read of other pfm files
				// 				end = strchr(p,0xa);
				// 				if(end)
				// 				{
				// 					end[0] = 0;
				// 
				// 					int	cx,cy;
				// 					if(sscanf(p,"%d %d",&cx,&cy) == 2)
				// 					{
				// 						*width = cx;
				// 						*height = cy;
				// 						p = &end[1];
				// 						end = strchr(p,0xa);
				// 						if(end)
				// 						{
				// 							return (end-Header)+1;
				// 						}
				// 					}
				// 				} //if (end)


			} //if(p)
		} //if(Header)
	}// if (len)
	return 0;
}

bool CPFMAccess::SetSize(int width, int height)
{
	if (m_width*m_height == width*height)
		return true;

	if (m_data != NULL)
		delete m_data;
	m_width = width;
	m_height = height;
	m_data = new float[3 * width*height];
	for (int i = 0; i<m_width; i++)
		for (int j = 0; j<m_height; j++)
		{
			m_data[3 * (i*m_height + j) + 0] = 0;
			m_data[3 * (i*m_height + j) + 1] = 0;
			m_data[3 * (i*m_height + j) + 2] = 0;
		}
	return true;
}

bool CPFMAccess::SetData(float*data)
{
	if (m_width == -1 || m_height == -1)
		return false;
	memcpy(m_data, data, 3 * sizeof(float)*m_height*m_width);
	return true;
}

void CPFMAccess::SetPixelValue(int x, int y, const float r, const float g, const float b)
{
	m_data[0 + 3 * x + 3 * y*m_width] = r;
	m_data[1 + 3 * x + 3 * y*m_width] = g;
	m_data[2 + 3 * x + 3 * y*m_width] = b;
}

void CPFMAccess::SetPixelValue(int x, int y, const float*value)
{
	for (int i = 0; i<3; i++)
		m_data[i + 3 * x + 3 * y*m_width] = value[i];
}

void CPFMAccess::SetPixelRValue(int x, int y, const float r)
{
	m_data[3 * x + 3 * y*m_width] = r;
}

void CPFMAccess::SetPixelGValue(int x, int y, const float g)
{
	m_data[1 + 3 * x + 3 * y*m_width] = g;
}


void CPFMAccess::SetPixelBValue(int x, int y, const float b)
{
	m_data[2 + 3 * x + 3 * y*m_width] = b;
}

void CPFMAccess::SetPixelComponentValue(int x, int y, const int component_id, const float v)
{
	if (component_id<0 || component_id>2)
		return;
	m_data[component_id + 3 * x + 3 * y*m_width] = v;
}


void CPFMAccess::GetPixelValue(int x, int y, float *value)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		value[0] = value[1] = value[2] = 0.0;
		return;
	}
	for (int i = 0; i<3; i++)
		value[i] = m_data[i + 3 * x + 3 * y*m_width];
}

void CPFMAccess::GetPixelValue(int x, int y, float* r, float* g, float* b)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		*r = *g = *b = 0.0;
		return;
	}
	*r = m_data[0 + 3 * x + 3 * y*m_width];
	*g = m_data[1 + 3 * x + 3 * y*m_width];
	*b = m_data[2 + 3 * x + 3 * y*m_width];
}

void CPFMAccess::GetPixelRValue(int x, int y, float *value)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		*value = 0.0;
		return;
	}
	*value = m_data[0 + 3 * x + 3 * y*m_width];
}

void CPFMAccess::GetPixelGValue(int x, int y, float *value)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		*value = 0.0;
		return;
	}
	*value = m_data[1 + 3 * x + 3 * y*m_width];
}

void CPFMAccess::GetPixelBValue(int x, int y, float *value)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		*value = 0.0;
		return;
	}
	*value = m_data[2 + 3 * x + 3 * y*m_width];
}

void CPFMAccess::GetPixelComponentValue(int x, int y, int component_id, float *value)
{
	if (x<0 || x >= m_width || y<0 || y >= m_height)
	{
		*value = 0.0;
		return;
	}
	if (component_id<0 || component_id>2)
	{
		*value = 0.0;
		return;
	}
	*value = m_data[component_id + 3 * x + 3 * y*m_width];
}

// contributed by Mao
Mat GetGrayImage(char* imgpath, int width, int height, float* thresh)
{
	CPFMAccess b;
	b.LoadFromFile(imgpath);

	float R = 0;
	float G = 0;
	float B = 0;
	vector<float> obj;
	Mat img(height, width, CV_32FC3);
	Mat img_gray(height, width, CV_32F);
	// get intensity map
	for (int row = 0; row<height; row++)
	{
		for (int col = 0; col < width; col++)
		{

			b.GetPixelBValue(col, row, &B);
			b.GetPixelGValue(col, row, &G);
			b.GetPixelRValue(col, row, &R);
			//img.at<Vec3f>(row, col)[0] = 65536 * B;
			//img.at<Vec3f>(row, col)[1] = 65536 * G;
			//img.at<Vec3f>(row, col)[2] = 65536 * R;
			float temp = R*0.299 + G*0.587 + B * 0.114;
			img_gray.at<float>(row, col) = temp;
			obj.push_back(temp);
		}
	}
	//namedWindow("img", WINDOW_AUTOSIZE);
	//imshow("img", img);
	sort(obj.begin(), obj.end());
	*thresh = obj[obj.size() / 10];
	return img_gray;
}

Mat GetRGBImage(char* imgpath)
{
	CPFMAccess b;
	b.LoadFromFile(imgpath);

	float R = 0;
	float G = 0;
	float B = 0;

	Mat img(b.m_height, b.m_width, CV_32FC3);
	Mat img_gray(b.m_height, b.m_width, CV_32FC1);
	// get intensity map
	for (int row = 0; row<b.m_height; row++)
	{
		for (int col = 0; col < b.m_width; col++)
		{

			b.GetPixelBValue(col, row, &B);
			b.GetPixelGValue(col, row, &G);
			b.GetPixelRValue(col, row, &R);
			img.at<Vec3f>(row, col)[0] = 255 * B;
			img.at<Vec3f>(row, col)[1] = 255 * G;
			img.at<Vec3f>(row, col)[2] = 255 * R;
			img_gray.at<float>(row, col) = R*0.299 + G*0.587 + B * 0.114;
		}
	}
	//namedWindow("img", WINDOW_AUTOSIZE);
	//imshow("img", img);
	return img;
}

Mat GetSourcelight(char* lamberpath, char* specpath, char* imgpath, int width, int height, float *Intensity)
{
	float thresh;
	Mat img_gray;
	img_gray = GetGrayImage(imgpath, width, height, &thresh);
	Mat spec_mask;
	spec_mask = imread(specpath, CV_LOAD_IMAGE_GRAYSCALE);
	Mat lamber_mask;
	lamber_mask = imread(lamberpath, CV_LOAD_IMAGE_GRAYSCALE);

	// get info from the brightest point
	float gray = 0.0;
	float max_intensity = 0.0;
	int max_intensity_row = 0;
	int max_intensity_col = 0;
	vector<int> row_v;
	vector<int> col_v;

	for (int row = 0; row < height; row++)//search whole spec image 
	{
		for (int col = 0; col < width; col++)
		{
			gray = img_gray.at<float>(row, col);
			if (spec_mask.at<uchar>(row, col)>128)
			{

				if (gray > max_intensity)
				{
					max_intensity = gray;
					max_intensity_row = row;//x
					max_intensity_col = col;//y
				}
				//get index of the mask area
				row_v.push_back(row);
				col_v.push_back(col);
			}
		}
	}

	/* get center position */
	double row_sum = 0.0;
	for (vector<int>::iterator iter = row_v.begin(); iter != row_v.end(); ++iter) // 
	{
		row_sum += (double)*iter;
	}
	double col_sum = 0.0;
	for (vector<int>::iterator iter = col_v.begin(); iter != col_v.end(); ++iter) // 
	{
		col_sum += (double)*iter;
	}

	double center_row = row_sum / (double)row_v.size();
	double center_col = col_sum / (double)col_v.size();

	/*get radius */
	vector<int>::iterator biggest_row = max_element(row_v.begin(), row_v.end());
	//cout << "Max element is " << *biggest_row << " at position " << distance(row_v.begin(), biggest_row) << endl;
	int edge_pix_row = *biggest_row;
	int edge_pix_col = col_v[distance(row_v.begin(), biggest_row)];
	//cout << edge_pix_row <<endl << edge_pix_col<<endl;
	double radius = sqrt(pow(abs(center_row - (double)edge_pix_row), 2) + pow(abs(center_col - (double)edge_pix_col), 2));
	//cout << radius << endl;

	/*calculate light direction*/
	// get Normal for the brightest point
	Mat R(1, 3, CV_64F);//reflection direction(vision direction)
	R.at<double>(0, 0) = 0.0;
	R.at<double>(0, 1) = 0.0;
	R.at<double>(0, 2) = 1.0;
	Mat N(1, 3, CV_64F);
	N.at<double>(0, 0) = ((double)max_intensity_row - (double)center_row) / radius;
	N.at<double>(0, 1) = ((double)max_intensity_col - (double)center_col) / radius;
	N.at<double>(0, 2) = sqrt(1 - N.at<double>(0, 0)*N.at<double>(0, 0) - N.at<double>(0, 1)*N.at<double>(0, 1));
	//cout << "N: " << N << endl;
	//for (int i = 0; i < 3; i++)
	// cout << N.at<double>(0, i) << endl;
	Mat L(1, 3, CV_64F);
	double p = R.dot(N)*(2.0);
	L = p*N - R;
	normalize(L, L);
	//cout << "L" << L << endl;
	/* get light intensity*/
	max_intensity = 0.0;
	for (int row = 0; row < height; row++)//search whole image 
	{
		for (int col = 0; col < width; col++)
		{

			if (lamber_mask.at<uchar>(row, col)>128)
			{
				gray = img_gray.at<float>(row, col);
				if (gray > max_intensity)
				{
					max_intensity = gray;
					max_intensity_row = row;
					max_intensity_col = col;

				}
				//get index of the mask area
				//row_v.push_back(row);
				//col_v.push_back(col);
			}
		}
	}
	L.at<double>(0, 0) = L.at<double>(0, 0);
	L.at<double>(0, 1) = L.at<double>(0, 1);
	L.at<double>(0, 2) = L.at<double>(0, 2);
	*Intensity = (float)max_intensity;
	cout << "max_intensity:  " << max_intensity << endl;
	return L;
	//return L;

}

Mat RerenderImg(Mat Normalmap, Mat Albedomap, Mat lightsource, Mat objmask, int height, int width)
{
	Mat new_obj(height, width, CV_32FC1, Scalar(0));
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (objmask.at<unsigned char>(i, j) > 128)
			{
				float prod = 0;
				prod = Normalmap.at<Vec3f>(i, j).dot(lightsource);
				new_obj.at<float>(i, j) = Albedomap.at<float>(i, j)*prod;
				if (new_obj.at<float>(i, j) < 0)
					new_obj.at<float>(i, j) = 0;
			}
		}
	}
	return new_obj;

}

