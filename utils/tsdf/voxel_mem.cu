#include <cassert>

#include "utils/cuda/errors.cuh"
#include "utils/tsdf/voxel_mem.cuh"

__global__ static void heap_init(int *heap) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < NUM_BLOCK) {
    heap[idx] = idx;
  }
}

__device__ __host__ Vector3<short> point2block(const Vector3<short> &point) {
  return point >> BLOCK_LEN_BITS;
}

__device__ __host__ Vector3<short> point2offset(const Vector3<short> &point) {
  return point & (BLOCK_LEN - 1);
}

__device__ __host__ unsigned int offset2index(const Vector3<short> &point_offset) {
  return point_offset.x + point_offset.y * BLOCK_LEN + point_offset.z * BLOCK_AREA;
}

VoxelMemPool::VoxelMemPool() {
  // initialize free block counter
  CUDA_SAFE_CALL(cudaMallocManaged(&num_free_blocks_, sizeof(int)));
  *num_free_blocks_ = NUM_BLOCK;
  // intialize voxel data buffer
  CUDA_SAFE_CALL(cudaMalloc(&voxels_rgbw_, sizeof(VoxelRGBW) * NUM_BLOCK * BLOCK_VOLUME));
  CUDA_SAFE_CALL(cudaMalloc(&voxels_tsdf_, sizeof(VoxelTSDF) * NUM_BLOCK * BLOCK_VOLUME));
  CUDA_SAFE_CALL(cudaMalloc(&voxels_segm_, sizeof(VoxelSEGM) * NUM_BLOCK * BLOCK_VOLUME));
  // initialize heap array
  CUDA_SAFE_CALL(cudaMalloc(&heap_, sizeof(int) * NUM_BLOCK));
  heap_init<<<NUM_BLOCK / 256, 256>>>(heap_);
  CUDA_CHECK_ERROR;
  CUDA_SAFE_CALL(cudaDeviceSynchronize());
}

void VoxelMemPool::ReleaseMemory() {
  CUDA_SAFE_CALL(cudaFree(voxels_rgbw_));
  CUDA_SAFE_CALL(cudaFree(voxels_tsdf_));
  CUDA_SAFE_CALL(cudaFree(voxels_segm_));
  CUDA_SAFE_CALL(cudaFree(num_free_blocks_));
  CUDA_SAFE_CALL(cudaFree(heap_));
}

__device__ int VoxelMemPool::AquireBlock() {
  const int idx = atomicSub(num_free_blocks_, 1);
  assert(idx >= 1);

  const VoxelBlock block(heap_[idx - 1]);

  #pragma unroll
  for (int i = 0; i < BLOCK_VOLUME; ++i) {
    VoxelRGBW &voxel_rgbw = GetVoxel<VoxelRGBW>(i, block);
    VoxelTSDF &voxel_tsdf = GetVoxel<VoxelTSDF>(i, block);
    VoxelSEGM &voxel_segm = GetVoxel<VoxelSEGM>(i, block);
    voxel_rgbw.weight = 0;
    voxel_tsdf.tsdf = 1;
    voxel_segm.probability = .1;
  }

  return block.idx;
}

__device__ void VoxelMemPool::ReleaseBlock(const int block_idx) {
  const int idx = atomicAdd(num_free_blocks_, 1);
  assert(idx < NUM_BLOCK);

  heap_[idx] = block_idx;
}

__device__ __host__ int VoxelMemPool::NumFreeBlocks() const {
  return *num_free_blocks_;
}

