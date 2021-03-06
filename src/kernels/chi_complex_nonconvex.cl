/*
 * chi_complex_mpmconvex.cl
 *
 *  Created on: Oct 24, 2012
 *      Author: bkloppenborg
 *  
 *  Description:
 *      OpenCL kernel that computes the chi elements for complex quantities
 *      under the assumption that the probability distributions is non-convex.
 */

/* 
 * Copyright (c) 2012 Brian Kloppenborg
 *
 * If you use this software as part of a scientific publication, please cite as:
 *
 * Kloppenborg, B.; Baron, F. (2012), "LibOI: The OpenCL Interferometry Library" 
 * (Version X). Available from  <https://github.com/bkloppenborg/liboi>.
 *
 * This file is part of the OpenCL Interferometry Library (LIBOI).
 * 
 * LIBOI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * as published by the Free Software Foundation, either version 3 
 * of the License, or (at your option) any later version.
 * 
 * LIBOI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with LIBOI.  If not, see <http://www.gnu.org/licenses/>.
 */

// The following define should be created during the kernel compilation on the host.
// but we initialize it here just in case.
#ifndef PI
#define PI 3.141592653589793
#endif

void complex_to_oi(float real, float imag, float * amp, float * phase);
void complex_to_oi(float real, float imag, float * amp, float * phase)
{
    (*amp) = sqrt(real * real + imag * imag);
    (*phase) = atan2(imag, real);
}

/// The chi_bispectra_convex kernel computes the chi elements for the amplitude and
/// phase of the bispectra.  
__kernel void chi_complex_nonconvex(
    __global float * data,
    __global float * data_err,
    __global float * model,
    __global float * output,
    __private unsigned int start,
    __private unsigned int n)
{
    size_t i = get_global_id(0);
    size_t index = start + i;
    
    float data_amp = 0;
    float data_phi = 0;
    float data_amp_err = data_err[index];
    float data_phi_err = data_err[n+index];
    float model_amp = 0;
    float model_phi = 0;
    
    // Retreive the amplitude and phase for the data and model.
    complex_to_oi(data[index], data[n+index], &data_amp, &data_phi);
    complex_to_oi(model[index], model[n+index], &model_amp, &model_phi);

    // Store the result:
    if(i < n)
    {
        output[index] = (data_amp - model_amp) / data_amp_err;
        output[n+index] = fmod(data_phi - model_phi, (float)PI) / data_phi_err;   
    }   
}