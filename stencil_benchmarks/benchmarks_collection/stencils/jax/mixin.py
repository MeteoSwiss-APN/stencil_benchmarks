# Stencil Benchmarks
#
# Copyright (c) 2017-2020, ETH Zurich and MeteoSwiss
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# SPDX-License-Identifier: BSD-3-Clause
import time
import warnings

from stencil_benchmarks.benchmark import Benchmark, Parameter


class StencilMixin(Benchmark):
    dry_runs = Parameter('stencil dry-runs before the measurement', 0)

    def setup(self):
        super().setup()

        if self.verify and self.dry_runs:
            warnings.warn(
                'using --dry-runs together with verification might lead to '
                'false negatives for stencils with read-write fields')

    def run_stencil(self, data):
        from jax import numpy as jnp
        device_data = [jnp.array(d, dtype=d.dtype) for d in data]
        for _ in range(self.dry_runs):
            device_data = self.stencil(*device_data)
        for dd in device_data:
            dd.block_until_ready()
        start = time.perf_counter()
        device_data = self.stencil(*device_data)
        for dd in device_data:
            dd.block_until_ready()
        end = time.perf_counter()
        for dd, d in zip(device_data, data):
            d[...] = dd[...]
        return end - start