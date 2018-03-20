
#include "CPFMAccess.h"


int main()
{
	const int kNUM_PIC = 21;// number of pictures
	const char* kObj = "Elephant";// Options: Pear Apple Elephant

	vector<Mat> img_gray_vec;
	vector<Mat> light_vec;
	vector<float> intens_vec;
	vector<float> shadow_thresh_vec;
	// Getting width and height 
	CPFMAccess img1;
	char* Img1Path = new char[200];//define sample image full path
	sprintf(Img1Path, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\image001.pbm", kObj);
	img1.LoadFromFile(Img1Path);
	int width = img1.m_width;
	int height = img1.m_height;

	/* loading and organize dataset into memory */
	char* LamberPath = new char[200];
	char* SpecPath = new char[200];
	sprintf(LamberPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\mask_I.png", kObj);
	sprintf(SpecPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\mask_dir_2.png", kObj);
	for (int i = 0; i < kNUM_PIC; i++)
	{
		//int i = 9;
		char* temppath = new char[200];
		float thresh;
		float intensity;
		sprintf(temppath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\image%03d.pbm",kObj, i + 1);
		Mat img_gray;
		img_gray = GetGrayImage(temppath, width, height, &thresh);
		img_gray_vec.push_back(img_gray);
		Mat L = GetSourcelight(LamberPath, SpecPath, temppath, width, height, &intensity);
		light_vec.push_back(L);
		intens_vec.push_back(intensity);
		shadow_thresh_vec.push_back(thresh);
		cout << i << endl;
		delete[] temppath;
		temppath = NULL;
	}


	/* method 1 getting Normal for each pixels in the picture */
	Mat obj_mask;
	int NUM_PIC_I = 10;
	char* ObjPath = new char[200];
	sprintf(ObjPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\mask.png", kObj);
	obj_mask = imread(ObjPath, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Albedo(height, width, CV_32FC1, Scalar(0));
	Mat Normal(height, width, CV_32FC3, Scalar(-1, -1, -1));
	Mat Normal_v(height, width, CV_32FC3, Scalar(-1, -1, -1));
	double Albdo;
	double Albedo_max = 0;
	Mat L_inv(NUM_PIC_I, 3, CV_64F);
	for (int i = 0; i<height; i++)
	{
		for (int j = 0; j<width; j++)
		{
			if (obj_mask.at<unsigned char>(i, j)>128)//for all obj pixel
			{

				Mat I(NUM_PIC_I, 1, CV_64FC1);
				Mat N(3, 1, CV_64FC1);
				Mat L(NUM_PIC_I, 3, CV_64FC1);
				/*I.at<double>(0, 0) = (double)img1_gray.at<float>(i, j);
				I.at<double>(1, 0) = (double)img2_gray.at<float>(i, j);
				I.at<double>(2, 0) = (double)img3_gray.at<float>(i, j);
				I.at<double>(3, 0) = (double)img4_gray.at<float>(i, j);
				I.at<double>(4, 0) = (double)img5_gray.at<float>(i, j);*/
				/* get qualified I */
				int count = 0;
				int pic = 0;
				while (count<NUM_PIC_I)
				{
					if (pic == kNUM_PIC)//if the point cannot find 3 qualified point 
					{
						break;
					}
					if (img_gray_vec[pic].at<float>(i, j) > shadow_thresh_vec[pic])
					{
						I.at<double>(count, 0) = (double)img_gray_vec[pic].at<float>(i, j) / (double)intens_vec[pic];
						L.at<double>(count, 0) = light_vec[pic].at<double>(0, 0);
						L.at<double>(count, 1) = light_vec[pic].at<double>(0, 1);
						L.at<double>(count, 2) = light_vec[pic].at<double>(0, 2);
						pic += 1;
						count += 1;
					}
					else
					{
						pic += 1;
					}
				}//end while

				if (pic == kNUM_PIC)
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
					N = N / Albdo;
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

	// save RGB transfered Albedo
	char* OutAlbdoPath = new char[200];
	sprintf(OutAlbdoPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\Albedo.png", kObj);
	imwrite(OutAlbdoPath, Albedo);
	delete[] OutAlbdoPath;
	OutAlbdoPath = NULL;

	//namedWindow("Albedo map", WINDOW_AUTOSIZE);
	//imshow("Albedo map", Albedo);
	//namedWindow("Normal map", WINDOW_AUTOSIZE);
	//imshow("Normal map", Normal);
	Mat Normal_U(600, 800, CV_8UC3);
	Normal = Normal * 255;
	Normal.convertTo(Normal_U, CV_8UC3);

	// save Normal to Normal path
	char* OutNormalPath = new char[200];
	sprintf(OutNormalPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\Normal.png", kObj);
	imwrite(OutNormalPath, Normal);
	delete[] OutNormalPath;
	OutNormalPath = NULL;


	/* get rerendered picture using light source same direction with the viewing direction*/
	Mat new_obj(height, width, CV_32FC1, Scalar(0));
	float m[] = { 0,0,1 };
	Mat lightsource(3, 1, CV_32FC1, m);
	new_obj = RerenderImg(Normal_v, Albedo, lightsource, obj_mask, height, width);
	//namedWindow("new img", WINDOW_AUTOSIZE);
	//imshow("new img", new_obj);
	
	// save new object 
	char* OutNewObjPath = new char[200];// path for 
	sprintf(OutNewObjPath, "D:\\resume_proj\\Civil7453Proj1\\Civil7453\\Civil7453\\%s\\new_Obj.png", kObj);
	imwrite(OutNewObjPath, new_obj);
	delete[] OutNewObjPath;
	OutNewObjPath = NULL;

	waitKey(0);
	return 0;
}