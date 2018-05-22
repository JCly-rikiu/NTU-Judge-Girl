#include <CL/cl.h>
#include <assert.h>
#include <inttypes.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>

const int MAXGPU = 3;
const int MAXK = 1024;
const int MAXBUFFER = 16777216;
const int NUMPERBLOCK = 128;

int main(int argc, char *argv[]) {
  cl_int status;
  cl_platform_id platform_id;
  cl_uint platform_id_got;
  status = clGetPlatformIDs(1, &platform_id, &platform_id_got);

  cl_device_id GPU[MAXGPU];
  cl_uint GPU_id_got;
  status =
      clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);

  cl_context context =
      clCreateContext(NULL, 1, GPU, NULL, NULL, &status);

  cl_command_queue commandQueue =
      clCreateCommandQueue(context, GPU[0], 0, &status);

  FILE *kernelfp = fopen("vecdot.cl", "r");
  char kernelBuffer[MAXK];
  const char *constKernelSource = kernelBuffer;
  size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);
  cl_program program = clCreateProgramWithSource(context, 1, &constKernelSource,
                                                 &kernelLength, &status);

  status = clBuildProgram(program, 1, GPU, NULL, NULL, NULL);

  cl_kernel kernel = clCreateKernel(program, "vecdot", &status);

  cl_uint *A = (cl_uint *)malloc(MAXBUFFER * sizeof(cl_uint));

  cl_mem bufferA =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     MAXBUFFER * sizeof(cl_uint), A, &status);

  int N;
  uint32_t key1, key2;
  while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3) {
    status = clSetKernelArg(kernel, 0, sizeof(uint32_t), (void *)&N);
    status = clSetKernelArg(kernel, 1, sizeof(int), (void *)&NUMPERBLOCK);
    status = clSetKernelArg(kernel, 2, sizeof(uint32_t), (void *)&key1);
    status = clSetKernelArg(kernel, 3, sizeof(uint32_t), (void *)&key2);
    status = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&bufferA);

    int globalNum = (N - 1) / NUMPERBLOCK + 1;
    size_t globalThreads[] = {((globalNum - 1) / 128 + 1) * 128};
    size_t localThreads[] = {128};
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalThreads, localThreads, 0, NULL, NULL);

    clEnqueueReadBuffer(commandQueue, bufferA, CL_TRUE, 0, globalNum * sizeof(cl_uint),
                        A, 0, NULL, NULL);

    uint32_t sum = 0;
    for (int i = 0; i < globalNum; i++)
      sum += A[i];
    printf("%" PRIu32 "\n", sum);
  }

  free(A);
  clReleaseContext(context);
  clReleaseCommandQueue(commandQueue);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseMemObject(bufferA);
  return 0;
}
