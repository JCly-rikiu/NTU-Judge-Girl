#include <CL/cl.h>
#include <assert.h>
#include <inttypes.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>

const int MAXGPU = 3;
const int MAXK = 1024;
const int MAXBUFFER = 67108864;
const int NUMPERTHREAD = 512;
const int BLOCKSIZE = 512;

int main(int argc, char *argv[]) {
  cl_int status;
  cl_platform_id platform_id;
  cl_uint platform_id_got;
  status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
  assert(status == CL_SUCCESS);

  cl_device_id GPU[MAXGPU];
  cl_uint GPU_id_got;
  status =
      clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);
  assert(status == CL_SUCCESS);

  cl_context context = clCreateContext(NULL, 1, GPU, NULL, NULL, &status);
  assert(status == CL_SUCCESS);

  cl_command_queue commandQueue =
      clCreateCommandQueue(context, GPU[0], 0, &status);
  assert(status == CL_SUCCESS);

  FILE *kernelfp = fopen("vecdot.cl", "r");
  char kernelBuffer[MAXK];
  const char *constKernelSource = kernelBuffer;
  size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);
  cl_program program = clCreateProgramWithSource(context, 1, &constKernelSource,
                                                 &kernelLength, &status);
  assert(status == CL_SUCCESS);

  status = clBuildProgram(program, 1, GPU, NULL, NULL, NULL);
  // assert(status == CL_SUCCESS);
  if (status != CL_SUCCESS) {
    char program_log[4096];
    size_t len;
    clGetProgramBuildInfo(program, GPU[0], CL_PROGRAM_BUILD_LOG, 4096,
                          program_log, &len);
    program_log[len] = '\0';
    printf("%s", program_log);

    abort();
  }

  cl_kernel kernel = clCreateKernel(program, "vecdot", &status);
  assert(status == CL_SUCCESS);

  cl_uint *A = (cl_uint *)malloc(MAXBUFFER / NUMPERTHREAD / BLOCKSIZE * sizeof(cl_uint));

  cl_mem bufferA =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                     MAXBUFFER / NUMPERTHREAD / BLOCKSIZE * sizeof(cl_uint), A, &status);
  assert(status == CL_SUCCESS);

  int N;
  uint32_t key1, key2;
  while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3) {
    status = clSetKernelArg(kernel, 0, sizeof(uint32_t), (void *)&N);
    status = clSetKernelArg(kernel, 1, sizeof(int), (void *)&NUMPERTHREAD);
    status = clSetKernelArg(kernel, 2, sizeof(uint32_t), (void *)&key1);
    status = clSetKernelArg(kernel, 3, sizeof(uint32_t), (void *)&key2);
    status = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&bufferA);
    status = clSetKernelArg(kernel, 5, BLOCKSIZE * sizeof(cl_uint), NULL);

    int globalNum = (N - 1) / NUMPERTHREAD + 1;
    int groupNum = (globalNum - 1) / BLOCKSIZE + 1;
    size_t globalThreads[] = {groupNum * BLOCKSIZE};
    size_t localThreads[] = {BLOCKSIZE};
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
                                    globalThreads, localThreads, 0, NULL, NULL);
    assert(status == CL_SUCCESS);

    status = clEnqueueReadBuffer(commandQueue, bufferA, CL_TRUE, 0,
                                 groupNum * sizeof(cl_uint), A, 0, NULL, NULL);
    assert(status == CL_SUCCESS);

    uint32_t sum = 0;
    for (int i = 0; i < groupNum; i++) sum += A[i];
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
