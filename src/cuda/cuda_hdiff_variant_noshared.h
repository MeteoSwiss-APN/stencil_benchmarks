#pragma once

#include "hdiff_stencil_variant.h"

namespace platform {

    namespace cuda {

        template <typename ValueType>
        __global__ void kernel_hdiff_noshared(ValueType const *in,
            ValueType const *coeff,
            ValueType *out,
            const int isize,
            const int jsize,
            const int ksize,
            const int istride,
            const int jstride,
            const int kstride) {

            const int i = blockIdx.x * blockDim.x + threadIdx.x;
            const int j = blockIdx.y * blockDim.y + threadIdx.y;
            const int k = blockIdx.z * blockDim.z + threadIdx.z;
            int index = i * istride + j * jstride + k * kstride;

            if (i < isize && j < jsize && k < ksize) {
                ValueType lap_ij = 4 * __ldg(&in[index]) - __ldg(&in[index - istride]) - __ldg(&in[index + istride]) -
                                   __ldg(&in[index - jstride]) - __ldg(&in[index + jstride]);
                ValueType lap_imj = 4 * __ldg(&in[index - istride]) - __ldg(&in[index - 2 * istride]) -
                                    __ldg(&in[index]) - __ldg(&in[index - istride - jstride]) -
                                    __ldg(&in[index - istride + jstride]);
                ValueType lap_ipj = 4 * __ldg(&in[index + istride]) - __ldg(&in[index]) -
                                    __ldg(&in[index + 2 * istride]) - __ldg(&in[index + istride - jstride]) -
                                    __ldg(&in[index + istride + jstride]);
                ValueType lap_ijm = 4 * __ldg(&in[index - jstride]) - __ldg(&in[index - istride - jstride]) -
                                    __ldg(&in[index + istride - jstride]) - __ldg(&in[index - 2 * jstride]) -
                                    __ldg(&in[index]);
                ValueType lap_ijp = 4 * __ldg(&in[index + jstride]) - __ldg(&in[index - istride + jstride]) -
                                    __ldg(&in[index + istride + jstride]) - __ldg(&in[index]) -
                                    __ldg(&in[index + 2 * jstride]);

                ValueType flx_ij = lap_ipj - lap_ij;
                flx_ij = flx_ij * (__ldg(&in[index + istride]) - __ldg(&in[index])) > 0 ? 0 : flx_ij;

                ValueType flx_imj = lap_ij - lap_imj;
                flx_imj = flx_imj * (__ldg(&in[index]) - __ldg(&in[index - istride])) > 0 ? 0 : flx_imj;

                ValueType fly_ij = lap_ijp - lap_ij;
                fly_ij = fly_ij * (__ldg(&in[index + jstride]) - __ldg(&in[index])) > 0 ? 0 : fly_ij;

                ValueType fly_ijm = lap_ij - lap_ijm;
                fly_ijm = fly_ijm * (__ldg(&in[index]) - __ldg(&in[index - jstride])) > 0 ? 0 : fly_ijm;

                out[index] = in[index] - coeff[index] * (flx_ij - flx_imj + fly_ij - fly_ijm);
            }
        }

        template <class ValueType>
        class hdiff_variant_noshared final : public hdiff_stencil_variant<cuda, ValueType> {
          public:
            using value_type = ValueType;
            using platform = cuda;
            using allocator = typename platform::template allocator<value_type>;

            hdiff_variant_noshared(const arguments_map &args)
                : hdiff_stencil_variant<cuda, ValueType>(args), m_iblocksize(args.get<int>("i-blocksize")),
                  m_jblocksize(args.get<int>("j-blocksize")), m_kblocksize(args.get<int>("k-blocksize")) {
                if (m_iblocksize <= 0 || m_jblocksize <= 0)
                    throw ERROR("invalid block size");
                platform::limit_blocksize(m_iblocksize, m_jblocksize, m_kblocksize);
            }
            ~hdiff_variant_noshared() {}

            void setup() override {
                hdiff_stencil_variant<platform, value_type>::setup();

                auto prefetch = [&](const value_type *ptr) {
                    if (cudaMemPrefetchAsync(ptr - this->zero_offset(), this->storage_size() * sizeof(value_type), 0) !=
                        cudaSuccess)
                        throw ERROR("error in cudaMemPrefetchAsync");
                };
                prefetch(this->in());
                prefetch(this->coeff());
                prefetch(this->out());

                cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);

                if (cudaDeviceSynchronize() != cudaSuccess)
                    throw ERROR("error in cudaDeviceSynchronize");
            }

            void hdiff(unsigned int i) override {
                kernel_hdiff_noshared<<<blocks(), blocksize()>>>(this->in(),
                    this->coeff(),
                    this->out(),
                    this->isize(),
                    this->jsize(),
                    this->ksize(),
                    this->istride(),
                    this->jstride(),
                    this->kstride());
                if (cudaDeviceSynchronize() != cudaSuccess)
                    throw ERROR("error in cudaDeviceSynchronize");
            }

          private:
            dim3 blocks() const {
                return dim3((this->isize() + m_iblocksize - 1) / m_iblocksize,
                    (this->jsize() + m_jblocksize - 1) / m_jblocksize,
                    (this->ksize() + m_kblocksize - 1) / m_kblocksize);
            }

            dim3 blocksize() const { return dim3(m_iblocksize, m_jblocksize, m_kblocksize); }

            int m_iblocksize, m_jblocksize, m_kblocksize;
        };

    } // cuda

} // namespace platform
