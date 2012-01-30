/*
 * CRoutine_Chi2.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: bkloppenborg
 */

#include "CRoutine_Chi2.h"
#include <cstdio>

using namespace std;

CRoutine_Chi2::CRoutine_Chi2(cl_device_id device, cl_context context, cl_command_queue queue)
	:CRoutine_Reduce_Sum(device, context, queue)
{
	// Specify the source location for the kernel.
	mSource.push_back("chi2.cl");
	mChi2SourceID = mSource.size() - 1;

	mChi2Temp = NULL;
	mChi2Output = NULL;
}

CRoutine_Chi2::~CRoutine_Chi2()
{
	if(mChi2Temp) clReleaseMemObject(mChi2Temp);
	if(mChi2Output) clReleaseMemObject(mChi2Output);
}

float CRoutine_Chi2::Chi2(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	size_t global = (size_t) n;
	size_t local = 0;
	float sum = 0;

	// Get the maximum work-group size for executing the kernel on the device
	err = clGetKernelWorkGroupInfo(mKernels[mChi2KernelID], mDeviceID, CL_KERNEL_WORK_GROUP_SIZE , sizeof(size_t), &local, NULL);
	COpenCL::CheckOCLError("Failed to determine workgroup size for chi2 kernel.", err);

	// Set the arguments to our compute kernel
	err  = clSetKernelArg(mKernels[mChi2KernelID], 0, sizeof(cl_mem), &data);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 1, sizeof(cl_mem), &data_err);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 2, sizeof(cl_mem), &model_data);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 3, sizeof(cl_mem), &mChi2Temp);
	err |= clSetKernelArg(mKernels[mChi2KernelID], 4, sizeof(int), &n);
	COpenCL::CheckOCLError("Failed to set chi2 kernel arguments.", err);

	// Execute the kernel over the entire range of the data set
	err = clEnqueueNDRangeKernel(mQueue, mKernels[mChi2KernelID], 1, NULL, &global, NULL, 0, NULL, NULL);
	COpenCL::CheckOCLError("Failed to enqueue chi2 kernel.", err);

#ifdef DEBUG_VERBOSE
	// Copy back the data, model, and errors:
	Chi2_CPU(data, data_err, model_data, n);
	ComputeSum_CPU(mChi2Output, n);
#endif // DEBUG_VERBOSE

	// Now fire up the parallel sum kernel and return the output.
	sum = ComputeSum(true, mChi2Output, mChi2Temp, tmp_buff1, tmp_buff2);

	return sum;
}

float CRoutine_Chi2::Chi2_CPU(cl_mem data, cl_mem data_err, cl_mem model_data, int n)
{
	int err = 0;
	cl_float * cpu_data = new cl_float[n];
	err = clEnqueueReadBuffer(mQueue, data, CL_TRUE, 0, n * sizeof(cl_float), cpu_data, 0, NULL, NULL);
	cl_float * cpu_data_err = new cl_float[n];
	err = clEnqueueReadBuffer(mQueue, data_err, CL_TRUE, 0, n * sizeof(cl_float), cpu_data_err, 0, NULL, NULL);
	cl_float * cpu_model_data = new cl_float[n];
	err = clEnqueueReadBuffer(mQueue, model_data, CL_TRUE, 0, n * sizeof(cl_float), cpu_model_data, 0, NULL, NULL);

	// we do this verbose
	float sum = 0;
	float tmp = 0;
	for(int i = 0; i < n; i++)
	{
		tmp = (cpu_data[i] - cpu_model_data[i]) / cpu_data_err[i];
		printf("%i %f %f %e %e \n", i, cpu_data[i], cpu_model_data[i], cpu_data[i] - cpu_model_data[i], cpu_data_err[i]);
		sum += tmp * tmp;
	}

	printf("Chi2: %f\n", sum);

	delete[] cpu_data;
	delete[] cpu_data_err;
	delete[] cpu_model_data;

	return sum;
}

/// Initialize the Chi2 routine.  Note, this internally allocates some memory for computing a parallel sum.
void CRoutine_Chi2::Init(int num_elements)
{
	int err = CL_SUCCESS;

	// First initialize the base-class constructor:
	CRoutine_Reduce_Sum::Init(num_elements, true);

	// Now allocate some memory
	if(mChi2Temp == NULL)
		mChi2Temp = clCreateBuffer(mContext, CL_MEM_READ_WRITE, num_elements * sizeof(cl_float), NULL, &err);

	if(mChi2Output == NULL)
		mChi2Output = clCreateBuffer(mContext, CL_MEM_READ_WRITE, sizeof(cl_float), NULL, &err);

	// Read the kernel, compile it
	string source = ReadSource(mSource[mChi2SourceID]);
    BuildKernel(source, "chi2", mSource[mChi2SourceID]);
    mChi2KernelID = mKernels.size() - 1;
}
