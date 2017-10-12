#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <string>
#include "CPFMAccess.h"
#include "math.h"
using namespace std;
using namespace cv;

Mat get_gray_image(char* imgpath, int width, int height, float* thresh)
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

Mat get_RGB_image(char* imgpath)
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

Mat get_sourcelight(char* lamberpath, char* specpath, char* imgpath, int width, int height, float *Intensity)
{
	float thresh;
	Mat img_gray;
	img_gray = get_gray_image(imgpath, width, height, &thresh);
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
	cout << "max_intensity:  "<< max_intensity << endl;
	return L;
	//return L;

}

Mat rerender_img(Mat Normalmap,Mat Albedomap,Mat lightsource, Mat objmask, int height,int width)
{
	Mat new_obj(height, width, CV_32FC1,Scalar(0));
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

//Mat rerender_img_II(Mat Normalmap, Mat objmask, int height, int width)
//{
//	Mat new_obj;
//	int curindex = 0;
//	vector<int> index;
//	vector<int> index_rplus;
//	vector<int> index_cplus;
//	/*Mat index_row;
//	Mat index_col;*/
//	Mat dz;
//	for (int row = 0; row < height; row++)//search whole spec image 
//	{
//		for (int col = 0; col < width; col++)
//		{
//			if (objmask.at<unsigned char>(row+1,col)>128&& objmask.at<unsigned char>(row, col+1)>128)
//			{
//				index.push_back(row*width + col);
//				index_rplus.push_back((row+1)*width+col);
//				index_cplus.push_back(row *width + col+1);
//				dz.push_back(Normalmap.at<Vec3f>(row,col)[0]/ Normalmap.at<Vec3f>(row, col)[2]*(-1));
//				dz.push_back(Normalmap.at<Vec3f>(row, col)[1] / Normalmap.at<Vec3f>(row, col)[2] * (-1));
//			}
//		}
//
//	}
//	Mat parameter(dz.row,index.size(),CV_32F,Scalar(0.0));
//	for (int i = 0; i < index.size(); i++)
//	{
//		int row = index[i] / width;
//		int col = index[i] - row*width;
//		vector<int>::iterator iter = find(index.begin(), index.end(), index[i+1]);
//		if (iter == v.end())
//			cout << "ERROR!" << endl;
//		else               
//			cout << "the index of value " << (*iter) << " is " << std::distance(v.begin(), iter) << std::endl;
//		parameter.at<float>(2 * i, );
//		parameter.at<float>(2 * i+1, );
//
//	}
//
//
//	return new_obj;
//
//}




int main()
{
	const int num_pic = 21;
	//string foldpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple";
	//char* img1path = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple\\image001.pbm";
	//char* lamberpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple\\mask_I.png";
	//char* specpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple\\mask_dir_2.png";
	//char* objpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple\\applemask.png";

	//string foldpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Elephant";
	//char* img1path = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Elephant\\image001.pbm";
	//char* lamberpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Elephant\\mask_I.png";
	//char* specpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Elephant\\mask_dir_2.png";
	//char* objpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Elephant\\mask.png";

	string foldpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear";
    char* img1path = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\image001.pbm";
    char* lamberpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\mask_I.png";
    char* specpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\mask_dir_2.png";
    char* objpath = "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\pearmask.png";
	//char* imgpath[num_pic];
	vector<Mat> IMG_gray;
	vector<Mat> Light;
	vector<float> Intens;
	vector<float> shadow_thresh;
	CPFMAccess img1;
	img1.LoadFromFile(img1path);
	int width = img1.m_width;
	int height = img1.m_height;
	for (int i = 0; i < num_pic; i++)
	{
		//int i = 9;
		char* temppath = new char[200];
		float thresh;
		float Intensity;
		//sprintf(temppath, "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Apple\\image%03d.pbm", i + 1);
		sprintf(temppath, "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\image%03d.pbm", i + 1);
		//sprintf(temppath, "C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\image%03d.pbm", i + 1);
		Mat img_gray;
		img_gray = get_gray_image(temppath, width, height, &thresh);
		IMG_gray.push_back(img_gray);
		Mat L = get_sourcelight(lamberpath, specpath, temppath, width, height, &Intensity);
		Light.push_back(L);
		Intens.push_back(Intensity);
		shadow_thresh.push_back(thresh);
		cout << i << endl;
		delete[] temppath;
		temppath = NULL;
	}


	/* method 1*/
	Mat obj_mask;
	int num_pic_I = 10;
	obj_mask = imread(objpath, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Albedo(height, width, CV_32FC1, Scalar(0));
	Mat Normal(height, width, CV_32FC3, Scalar(-1, -1, -1));
	Mat Normal_v(height, width, CV_32FC3, Scalar(-1, -1, -1));
	double Albdo;
	double Albedo_max = 0;
	Mat L_inv(num_pic_I, 3, CV_64F);
	for (int i = 0; i<height; i++)
	{
		for (int j = 0; j<width; j++)
		{
			if (obj_mask.at<unsigned char>(i, j)>128)//for all obj pixel
			{

				Mat I(num_pic_I, 1, CV_64FC1);
				Mat N(3, 1, CV_64FC1);
				Mat L(num_pic_I, 3, CV_64FC1);
				/*I.at<double>(0, 0) = (double)img1_gray.at<float>(i, j);
				I.at<double>(1, 0) = (double)img2_gray.at<float>(i, j);
				I.at<double>(2, 0) = (double)img3_gray.at<float>(i, j);
				I.at<double>(3, 0) = (double)img4_gray.at<float>(i, j);
				I.at<double>(4, 0) = (double)img5_gray.at<float>(i, j);*/
				/* get qualified I */
				int count = 0;
				int pic = 0;
				while (count<num_pic_I)
				{
					if (pic == num_pic)//if the point cannot find 3 qualified point 
					{
						//Normal.at<Vec3f>(i, j)[2] = 0;
						//Normal.at<Vec3f>(i, j)[1] = 0;
						//Normal.at<Vec3f>(i, j)[0] = 0;
						break;
					}
					if (IMG_gray[pic].at<float>(i, j) > shadow_thresh[pic])
					{
						I.at<double>(count, 0) = (double)IMG_gray[pic].at<float>(i, j) / (double)Intens[pic];
						L.at<double>(count, 0) = Light[pic].at<double>(0, 0);
						L.at<double>(count, 1) = Light[pic].at<double>(0, 1);
						L.at<double>(count, 2) = Light[pic].at<double>(0, 2);
						pic += 1;
						count += 1;
					}
					else
					{
						pic += 1;
					}
				}//end while

				if (pic == num_pic)
				{
					continue;
				}
				else
				{

					invert(L, L_inv, DECOMP_SVD);
					N = L_inv*I;
					//cout << "L: " << L << endl;
					//cout << "L_inv: " << L_inv << endl;
					//cout << "I: " << I << endl;
					Albdo = norm(N, NORM_L2);
					//cout << "Albdo: "<< Albdo<<endl;
					if (Albdo > Albedo_max) Albedo_max = Albdo;
					N = N/Albdo;
					Normal_v.at<Vec3f>(i, j) = N;
					Albedo.at<float>(i, j) = (float)Albdo;
					Normal.at<Vec3f>(i, j)[2] = (float)((N.at<double>(0, 0) + 1.0) / 2.0);
					Normal.at<Vec3f>(i, j)[1] = (float)((N.at<double>(1, 0) + 1.0) / 2.0);
					Normal.at<Vec3f>(i, j)[0] = (float)((N.at<double>(2, 0) + 1.0) / 2.0);
					//cout << "R: "<<Normal.at<Vec3f>(i, j)[2]<<endl;
					//cout << "G: " << Normal.at<Vec3f>(i, j)[1] << endl;
					//cout << "B: " << Normal.at<Vec3f>(i, j)[0] << endl;
					//cout << Albdo << endl;
				}//end if
			}
		}

	}
	cout << "Albedo_max: " << Albedo_max << endl;
	Albedo = Albedo / (float)Albedo_max;
	Mat Albedo_U(600, 800, CV_8U);
	Albedo = Albedo * 255;
	Albedo.convertTo(Albedo_U, CV_8U);
	imwrite("C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\Albedo.png", Albedo);
	namedWindow("Albedo map", WINDOW_AUTOSIZE);
	imshow("Albedo map", Albedo);
	namedWindow("Normal map", WINDOW_AUTOSIZE);
	imshow("Normal map", Normal);
	Mat Normal_U(600, 800, CV_8UC3);
	Normal = Normal * 255;
	Normal.convertTo(Normal_U,CV_8UC3);
	imwrite("C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\Normal.png", Normal);


	/* get rerendered picture using light source same direction with the viewing direction*/
	



	Mat new_obj(height, width, CV_32FC1, Scalar(0));
	float m[] = { 0,0,1 };
	Mat lightsource(3,1,CV_32FC1,m);
	Normal = Normal;
	new_obj = rerender_img(Normal_v, Albedo, lightsource, obj_mask, height, width);
	namedWindow("new img", WINDOW_AUTOSIZE);
	imshow("new img", new_obj);
	imwrite("C:\\Users\\li.6794\\Desktop\\Assignment_1\\Assignment_1\\Pear\\new_obj.png",new_obj);
	/* Bounus 1 */
	//Mat new_obj_II(height, width, CV_32FC1, Scalar(0));
	//new_obj_II = rerender_img_II(Normal_v, obj_mask, height, width);

	//namedWindow("II", WINDOW_AUTOSIZE);
	//imshow("II", new_obj_II);


	waitKey(-1);
	return 0;
}