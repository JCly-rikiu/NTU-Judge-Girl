#include <CL/cl.h>
#include <stdio.h>

const int MAXGPU = 10;
const int MAXK = 4096;
const int MAXBUFFER = 4096;

int main() {
  cl_int status;
  cl_platform_id platform_id;
  cl_uint platform_id_got;
  status = clGetPlatformIDs(1, &platform_id, &platform_id_got);

  cl_device_id GPU[MAXGPU];
  cl_uint GPU_id_got;
  status =
      clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);

  cl_context context =
      clCreateContext(NULL, GPU_id_got, GPU, NULL, NULL, &status);

  cl_command_queue commandQueue =
      clCreateCommandQueue(context, GPU[0], 0, &status);

  char file_name[40];
  scanf("%s", file_name);
  FILE* kernelfp = fopen(file_name, "r");
  char kernelBuffer[MAXK];
  const char* constKernelSource = kernelBuffer;
  size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);
  cl_program program = clCreateProgramWithSource(context, 1, &constKernelSource,
                                                 &kernelLength, &status);

  status = clBuildProgram(program, GPU_id_got, GPU, NULL, NULL, NULL);

  char program_log[MAXBUFFER];
  size_t len;
  status = clGetProgramBuildInfo(program, GPU[0], CL_PROGRAM_BUILD_LOG, MAXBUFFER, program_log, &len);
  program_log[len] = '\0';
  printf("%s", program_log);

  clReleaseContext(context);
  clReleaseCommandQueue(commandQueue);
  clReleaseProgram(program);
  return 0;
}
