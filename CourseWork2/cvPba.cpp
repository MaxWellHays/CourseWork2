#include "cvPba.h"

Point2D cvPba::ConvertCvPoint(cv::Point2f &point)
{
	return Point2D(point.x, point.y);
}

cvPba::cvPba()
{
}

cvPba::~cvPba()
{
}



vector<Point3D_<float>> cvPba::generateRough3dPoints(vector<cv::Point2f>& points, int width, int height)
{
	vector<Point3D_<float>> result;
	result.reserve(points.size());

	for (auto& point : points)
	{
		result.push_back(Point3D());
		result.back().SetPoint(point.x * 2 / width, point.y * 2 / height, 10.0f);
	}
	return result;
}

float cvPba::getLockedMask(bool lockFocal, bool lockPosition, bool lockRotation, bool lockDistortion)
{
	return lockFocal*LOCK_FOCAL + lockPosition*LOCK_POSITION + lockRotation*LOCK_ROTATION + lockDistortion*LOCK_DISTORTION;
}

void cvPba::RunBundleAdjustment(vector<vector<cv::Point2f>>& imagePoints)
{
	vector<CameraT>        camera_data;    //camera (input/ouput)
	vector<Point3D>        point_data;     //3D point(iput/output)
	vector<Point2D>        measurements;   //measurment/projection vector
	vector<int>            camidx, ptidx;  //index of camera/point for each projection

	for (int i = 0; i < imagePoints[0].size(); ++i)
	{
		measurements.push_back(ConvertCvPoint(imagePoints[0][i]));
		measurements.push_back(ConvertCvPoint(imagePoints[1][i]));
		camidx.push_back(0);
		camidx.push_back(1);
		ptidx.push_back(i);
		ptidx.push_back(i);
	}

	ParallelBA pba;

	camera_data.resize(2);

	camera_data[0].SetFocalLength(900);
	float translation[3] = { 0, 0, 0 };
	camera_data[0].SetTranslation(translation);
	float rotation[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	camera_data[0].SetMatrixRotation(rotation);
	camera_data[0].constant_camera = getLockedMask(true);

	camera_data[1] = CameraT(camera_data[0]);
	translation[2] = -1;
	camera_data[1].SetTranslation(translation);

	camera_data[0].constant_camera = getLockedMask(true, true, true);

	pba.SetCameraData(camera_data.size(), &camera_data[0]);

	point_data = generateRough3dPoints(imagePoints[0], 750, 501);

	pba.SetPointData(point_data.size(), &point_data[0]);

	pba.SetProjection(measurements.size(), &measurements[0], &ptidx[0], &camidx[0]);

	while (pba.RunBundleAdjustment() > 30);

	camera_data[0].constant_camera = getLockedMask(false);
	camera_data[0].constant_camera = getLockedMask(false);

	while (pba.RunBundleAdjustment() > 30);
}