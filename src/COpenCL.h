/*
 * COpenCL.h
 *
 *  Created on: Nov 11, 2011
 *      Author: bkloppenborg
 *
 * Primary class to initialize an OpenCL context.
 * Manages the creation and deletion of OpenCL objects, programs.
 * Also implements fast-call routines to get commonly needed interferometric quantities.
 *
 */

#ifndef COPENCL_H_
#define COPENCL_H_

#pragma OPENCL EXTENSION CL_APPLE_gl_sharing : enable

// Enable OpenCL exceptions
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
	#include <OpenCL/cl.hpp>
#else
	#include <CL/cl.hpp>
#endif

#include <GL/gl.h>
#include <string>

using namespace std;

// TODO: Make this class support multiple devices.
// TODO: Permit the user to specify a specific OpenCL device to use.

class COpenCL
{
protected:
	// Datamembers for the OpenCL device, context and queue.
	// At present we only support one device/context/queue, but could add more in the future
	// TODO: Make this class support multiple OpenCL devices.
	vector<cl::Device> mDevices;
	cl::Context mContext;
	cl::CommandQueue mQueue;


public:
	COpenCL();
	virtual ~COpenCL();

public:

	static void CheckOCLError(string user_message, int error_code);

	cl_context		GetContext();
	void 			GetDeviceList(cl_platform_id platform, vector<cl_device_id> * devices);
	cl_device_type  GetDeviceType(cl_device_id device);
	void 			GetPlatformList(vector<cl_platform_id> * platforms);
	static string 	GetOCLErrorString(cl_int err);

	void Init(cl_device_type type);
	void Init(cl_device_id device_id);

	cl_device_id FindDevice(cl_device_type type);

	void PrintDeviceInfo(cl_device_id device_id);
	void PrintPlatformInfo(cl_platform_id platform_id);
};

#endif /* COPENCL_H_ */
