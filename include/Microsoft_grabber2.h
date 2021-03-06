/*
Copyright (C) 2012 Steven Hickson

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/
#pragma once
#include <pcl/pcl_config.h>

#ifndef __PCL_IO_MICROSOFT_GRABBER__
#define __PCL_IO_MICROSOFT_GRABBER__

#include <pcl/io/eigen.h>
#include <pcl/io/boost.h>
#include <pcl/io/grabber.h>
#include <string>
#include <deque>
#include <pcl/common/synchronizer.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/time.h>
#include <pcl/console/print.h>
#include <pcl/exceptions.h>
#include <iostream>

#include <assert.h>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <objbase.h>
#include "Kinect.h"

#include <opencv2/opencv.hpp>

namespace pcl
{
	struct PointXYZ;
	struct PointXYZRGB;
	struct PointXYZRGBA;
	struct PointXYZI;
	template <typename T> class PointCloud;
	class MatDepth : public cv::Mat { }; //I have to use this to get around the assuming code in registerCallback in grabber.h
	class KinectData {
	public:
		KinectData() { };
		KinectData(const cv::Mat &image_, const MatDepth &depth_, const PointCloud<PointXYZRGBA> &cloud_) : image(image_), depth(depth_), cloud(cloud_) { };

		pcl::PointCloud<pcl::PointXYZRGBA> cloud;
		cv::Mat image;
		MatDepth depth;
	};

	/** \brief Grabber for OpenNI devices (i.e., Primesense PSDK, Microsoft Kinect, Asus XTion Pro/Live)
	* \author Nico Blodow <blodow@cs.tum.edu>, Suat Gedikli <gedikli@willowgarage.com>
	* \ingroup io
	*/
	class PCL_EXPORTS Microsoft2Grabber : public Grabber
	{
	public:
		typedef boost::shared_ptr<Microsoft2Grabber> Ptr;
		typedef boost::shared_ptr<const Microsoft2Grabber> ConstPtr;

		typedef enum
		{
			Microsoft_Default_Mode = 0, // VGA@30Hz
			Microsoft_SXGA_15Hz = 1    // Need to fill the rest of this up
		} Mode;

		//define callback signature typedefs
		typedef void (sig_cb_microsoft_image) (const boost::shared_ptr<cv::Mat> &);
		typedef void (sig_cb_microsoft_depth_image) (const MatDepth &);
		typedef void (sig_cb_microsoft_image_depth_image) (const boost::shared_ptr<cv::Mat> &, const MatDepth &, float);
		typedef void (sig_cb_microsoft_point_cloud_rgba) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZRGBA> >&);
		typedef void (sig_cb_microsoft_all_data) (const boost::shared_ptr<const KinectData> &);
		/*typedef void (sig_cb_microsoft_ir_image) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZI> >&);
		typedef void (sig_cb_microsoft_point_cloud) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZ> >&);
		typedef void (sig_cb_microsoft_point_cloud_rgb) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZRGB> >&);
		typedef void (sig_cb_microsoft_point_cloud_i) (const boost::shared_ptr<const pcl::PointCloud<pcl::PointXYZI> >&);*/

		Microsoft2Grabber (const int instance = 0);
		//const Mode& depth_mode = OpenNI_Default_Mode,
		//const Mode& image_mode = OpenNI_Default_Mode);

		/** \brief virtual Destructor inherited from the Grabber interface. It never throws. */
		virtual ~Microsoft2Grabber () throw ();

		/** \brief Start the data acquisition. */
		virtual void
			start ();

		/** \brief Stop the data acquisition. */
		virtual void
			stop ();

		/** \brief Check if the data acquisition is still running. */
		virtual bool
			isRunning () const;

		virtual std::string
			getName () const;

		/** \brief Obtain the number of frames per second (FPS). */
		virtual float 
			getFramesPerSecond () const;

		/** \brief Get a boost shared pointer to the \ref OpenNIDevice object. */
		/*inline boost::shared_ptr<Microsoft2Grabber>
		getDevice () const;*/


		//Kinect Camera Settings
		ICoordinateMapper*      m_pCoordinateMapper;
		bool CameraSettingsSupported;

		void GetPointCloudFromData(const boost::shared_ptr<cv::Mat> &img, const MatDepth &depth, boost::shared_ptr<PointCloud<PointXYZRGBA>> &cloud, bool alignToColor, bool preregistered) const;

		//These should not be called except within the thread by the KinectCapture class process manager
		void ProcessThreadInternal();

		void SetLargeCloud() {
			m_largeCloud = true;
		}

		void SetNormalCloud() {
			m_largeCloud = false;
		}

	protected:
		boost::signals2::signal<sig_cb_microsoft_image>* image_signal_;
		boost::signals2::signal<sig_cb_microsoft_depth_image>* depth_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_image_depth_image>* image_depth_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_rgba>* point_cloud_rgba_signal_;
		boost::signals2::signal<sig_cb_microsoft_all_data>* all_data_signal_;
		/*boost::signals2::signal<sig_cb_microsoft_ir_image>* ir_image_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud>* point_cloud_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_i>* point_cloud_i_signal_;
		boost::signals2::signal<sig_cb_microsoft_point_cloud_rgb>* point_cloud_rgb_signal_;
		*/
		Synchronizer<boost::shared_ptr<cv::Mat>, MatDepth > rgb_sync_;

		bool m_depthStarted, m_videoStarted, m_audioStarted, m_infraredStarted, m_person, m_preregistered;
		// Current Kinect
		IKinectSensor*          m_pKinectSensor;

		//IColorFrameReader*      m_pColorFrameReader;
		IMultiSourceFrameReader*m_pMultiSourceFrameReader;

		static const int        cColorWidth  = 1920;
		static const int        cColorHeight = 1080;
		static const int        cDepthWidth  = 512;
		static const int        cDepthHeight = 424;
		cv::Size m_colorSize, m_depthSize;
		RGBQUAD* m_pColorRGBX;
		UINT16 *m_pDepthBuffer;
		ColorSpacePoint *m_pColorCoordinates;
		CameraSpacePoint *m_pCameraSpacePoints;
		cv::Mat m_colorImage, m_depthImage;
#define COLOR_PIXEL_TYPE CV_8UC4
#define DEPTH_PIXEL_TYPE CV_16UC1

		bool m_largeCloud;
		HANDLE hStopEvent, hKinectThread, hDepthMutex, hColorMutex;
		WAITABLE_HANDLE hFrameEvent;
		bool m_depthUpdated, m_colorUpdated, m_infraredUpdated, m_skeletonUpdated;
		LONGLONG m_rgbTime, m_depthTime, m_infraredTime;
		INT64 timestep;
		//boost::mutex m_depthMutex, m_colorMutex, m_infraredMutex;

		void Release();
		void GetNextFrame();
		void FrameArrived(IMultiSourceFrameArrivedEventArgs *pArgs);
		void DepthFrameArrived(IDepthFrameReference* pDepthFrameReference);
		void ColorFrameArrived(IColorFrameReference* pColorFrameReference);
		void BodyIndexFrameArrived(IBodyIndexFrameReference* pBodyIndexFrameReference);
		bool GetCameraSettings();

		void imageDepthImageCallback(const boost::shared_ptr<cv::Mat> &image, const MatDepth &depth_image);
		boost::shared_ptr<pcl::PointCloud<pcl::PointXYZRGBA> > convertToXYZRGBAPointCloud (const boost::shared_ptr<cv::Mat> &image,
			const MatDepth &depth_image) const;
		/** \brief Convert a Depth + RGB image pair to a pcl::PointCloud<PointT>
		* \param[in] image the RGB image to convert
		* \param[in] depth_image the depth image to convert
		*/
		/*template <typename PointT> typename pcl::PointCloud<PointT>::Ptr
		convertToXYZRGBPointCloud (const boost::shared_ptr<openni_wrapper::Image> &image,
		const boost::shared_ptr<openni_wrapper::DepthImage> &depth_image) const;*/
	};

}

#endif //__PCL_IO_MICROSOFT_GRABBER__