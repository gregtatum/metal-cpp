#include "mtlpp/mtlpp.hpp"
#include <algorithm> // std::min
#include <fstream>
#include <iostream>
#include <span>    // std::span
#include <stdio.h> // printf

/**
 * Performing Calculations on a GPU.
 *
 * This example is an implementation of the tutorial:
 * https://developer.apple.com/documentation/metal/basic_tasks_and_concepts/performing_calculations_on_a_gpu
 */
void
printSpan(const char* message, const std::span<float> container)
{
  std::cout << message;

  for (const auto& n : container) {
    std::cout << n << ' ';
  }
  std::cout << '\n';
}

int
main()
{
  const char* program =
#include "./simple-compute-pipeline.metal"
    ;
  printf("Find the default GPU for the system.\n");
  auto device = mtlpp::Device::CreateSystemDefaultDevice();

  printf("This creates functions, not sure why it's needed separate from the "
         "device.\n");
  auto library = device.NewLibrary(program, mtlpp::CompileOptions(), nullptr);

  printf("Create the function\n");
  auto function = library.NewFunction("add_arrays");

  printf("The pipeline stores a group of functions.\n");
  auto pipeline = device.NewComputePipelineState(function, nullptr);

  printf("Create a command queue to be able to send commands to the GPU\n");
  auto commandQueue = device.NewCommandQueue();
  auto commandBuffer = commandQueue.CommandBuffer();
  auto computeEncoder = commandBuffer.ComputeCommandEncoder();

  printf("Create the buffers that will be operated upon.\n");
  uint32_t bufferSize = 32;
  auto bufferA =
    device.NewBuffer(bufferSize, mtlpp::ResourceOptions::StorageModeShared);
  auto bufferB =
    device.NewBuffer(bufferSize, mtlpp::ResourceOptions::StorageModeShared);
  auto bufferResult =
    device.NewBuffer(bufferSize, mtlpp::ResourceOptions::StorageModeShared);

  printf("Initialize the numbers.\n");
  {
    float* dataA = static_cast<float*>(bufferA.GetContents());
    float* dataB = static_cast<float*>(bufferB.GetContents());
    for (auto i = 0; i < bufferSize; i++) {
      dataA[i] = static_cast<float>(i);
      dataB[i] = static_cast<float>(i * 0.5);
    }
  }

  printf("Hook up the buffers.\n");
  computeEncoder.SetComputePipelineState(pipeline);
  computeEncoder.SetBuffer(bufferA, 0, 0);
  computeEncoder.SetBuffer(bufferB, 0, 1);
  computeEncoder.SetBuffer(bufferResult, 0, 2);

  printf("Decide how many thread groups to use on the data.\n");
  auto threadGroupSize =
    std::min(bufferSize, pipeline.GetMaxTotalThreadsPerThreadgroup());

  printf("This will divide up the threads with the data.\n");
  computeEncoder.DispatchThreadgroups(mtlpp::Size(threadGroupSize, 1, 1),
                                      mtlpp::Size(bufferSize, 1, 1));

  printf("We are done configuring the encoder.\n");
  computeEncoder.EndEncoding();

  printf("Run the command.\n");
  commandBuffer.Commit();
  commandBuffer.WaitUntilCompleted();

  printf("Access the results.\n");
  float* a = static_cast<float*>(bufferA.GetContents());
  float* b = static_cast<float*>(bufferB.GetContents());
  float* result = static_cast<float*>(bufferResult.GetContents());

  std::span aSpan{ a, bufferSize };
  std::span bSpan{ b, bufferSize };
  std::span resultSpan{ result, bufferSize };

  printSpan("Buffer A: ", std::span{ a, bufferSize });
  printSpan("Buffer B: ", std::span{ b, bufferSize });
  printSpan("A + B: ", std::span{ result, bufferSize });
}
