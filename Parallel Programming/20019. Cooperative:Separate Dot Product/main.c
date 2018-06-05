#include <CL/cl.h>
#include <assert.h>
#include <inttypes.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>

const int MAXGPU = 3;
const int MAXK = 1024;
const int MAXBUFFER = 1073741824;
const int NUMPERTHREAD = 512;
const int BLOCKSIZE = 512;

#define MAX_INPUT 20000
int N[MAX_INPUT];
uint32_t key1[MAX_INPUT], key2[MAX_INPUT];
uint32_t ans[MAX_INPUT];

int main(int argc, char *argv[]) {
  cl_int status;
  cl_platform_id platform_id;
  cl_uint platform_id_got;
  status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
  assert(status == CL_SUCCESS);

  cl_device_id GPU[MAXGPU];
  cl_uint GPU_num;
  status =
      clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_num);
  assert(status == CL_SUCCESS);
  GPU_num = 2;

  FILE *kernelfp = fopen("vecdot.cl", "r");
  char kernelBuffer[MAXK];
  const char *constKernelSource = kernelBuffer;
  size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);

  cl_context context[MAXGPU];
  cl_command_queue commandQueue[MAXGPU];
  cl_program program[MAXGPU];
  cl_kernel kernel[MAXGPU];

  int total_size = MAXBUFFER / NUMPERTHREAD / BLOCKSIZE;
  cl_uint *A[3];
  cl_mem bufferA[3];
  for (int i = 0; i < GPU_num; i++) {
    context[i] = clCreateContext(NULL, 1, &GPU[i], NULL, NULL, &status);
    assert(status == CL_SUCCESS);
    commandQueue[i] = clCreateCommandQueue(context[i], GPU[i], 0, &status);
    assert(status == CL_SUCCESS);
    program[i] = clCreateProgramWithSource(context[i], 1, &constKernelSource,
                                           &kernelLength, &status);
    assert(status == CL_SUCCESS);
    status = clBuildProgram(program[i], 1, &GPU[i], NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
      char program_log[4096];
      size_t len;
      clGetProgramBuildInfo(program[i], GPU[i], CL_PROGRAM_BUILD_LOG, 4096,
                            program_log, &len);
      program_log[len] = '\0';
      printf("%s", program_log);

      abort();
    }
    kernel[i] = clCreateKernel(program[i], "vecdot", &status);
    assert(status == CL_SUCCESS);

    A[i] = (cl_uint *)malloc(total_size * sizeof(cl_uint));
    bufferA[i] =
        clCreateBuffer(context[i], CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                       total_size * sizeof(cl_uint), A[i], &status);
    assert(status == CL_SUCCESS);
  }

  int input_id = 0;
  while (scanf("%d %" PRIu32 " %" PRIu32, N + input_id, key1 + input_id,
               key2 + input_id) == 3)
    input_id++;

#pragma omp parallel for schedule(dynamic, 1) num_threads(2)
  for (int now = 0; now < input_id; now++) {
    int _N = N[now];
    int tid = omp_get_thread_num();
    status = clSetKernelArg(kernel[tid], 0, sizeof(uint32_t), (void *)&_N);
    status = clSetKernelArg(kernel[tid], 1, sizeof(int), (void *)&NUMPERTHREAD);
    status =
        clSetKernelArg(kernel[tid], 2, sizeof(uint32_t), (void *)&key1[now]);
    status =
        clSetKernelArg(kernel[tid], 3, sizeof(uint32_t), (void *)&key2[now]);
    status =
        clSetKernelArg(kernel[tid], 4, sizeof(cl_mem), (void *)&bufferA[tid]);
    status = clSetKernelArg(kernel[tid], 5, BLOCKSIZE * sizeof(cl_uint), NULL);

    int globalNum = (_N - 1) / NUMPERTHREAD + 1;
    int groupNum = (globalNum - 1) / BLOCKSIZE + 1;
    size_t globalThreads[] = {groupNum * BLOCKSIZE};
    size_t localThreads[] = {BLOCKSIZE};
    status = clEnqueueNDRangeKernel(commandQueue[tid], kernel[tid], 1, NULL,
                                    globalThreads, localThreads, 0, NULL, NULL);
    assert(status == CL_SUCCESS);

    status =
        clEnqueueReadBuffer(commandQueue[tid], bufferA[tid], CL_TRUE, 0,
                            groupNum * sizeof(cl_uint), A[tid], 0, NULL, NULL);
    assert(status == CL_SUCCESS);

    uint32_t sum = 0;
    for (int i = 0; i < groupNum; i++) sum += A[tid][i];
    ans[now] = sum;
  }

  for (int now = 0; now < input_id; now++) printf("%" PRIu32 "\n", ans[now]);

  for (int i = 0; i < GPU_num; i++) {
    free(A[i]);
    clReleaseMemObject(bufferA[i]);
    clReleaseCommandQueue(commandQueue[i]);
    clReleaseKernel(kernel[i]);
    clReleaseContext(context[i]);
    clReleaseProgram(program[i]);
  }

  return 0;
}
