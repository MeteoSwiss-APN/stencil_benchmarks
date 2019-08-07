import numpy as np

from . import base

# pylint: disable=no-member,invalid-name


class Copy1D(base.CopyStencil):
    def run_stencil(self, data_set):
        inp, out = self.inouts[data_set]
        out[:] = inp[:]


class Copy(base.CopyStencil):
    def run_stencil(self, data_set):
        inp, out = self.inouts[data_set]
        out[self.inner_slice()] = inp[self.inner_slice()]


class OnesidedAverage(base.OnesidedAverageStencil):
    def run_stencil(self, data_set):
        inp, out = self.inouts[data_set]
        shift = np.zeros(3, dtype=int)
        shift[self.axis] = 1
        out[self.inner_slice()] = (inp[self.inner_slice(shift)] +
                                   inp[self.inner_slice()]) / 2


class SymmetricAverage(base.SymmetricAverageStencil):
    def run_stencil(self, data_set):
        inp, out = self.inouts[data_set]
        shift = np.zeros(3, dtype=int)
        shift[self.axis] = 1
        out[self.inner_slice()] = (inp[self.inner_slice(shift)] +
                                   inp[self.inner_slice(-shift)]) / 2


class Laplacian(base.LaplacianStencil):
    def run_stencil(self, data_set):
        inp, out = self.inouts[data_set]
        along_axes = (self.along_x, self.along_y, self.along_z)
        coeff = 2 * sum(along_axes)
        out[self.inner_slice()] = coeff * inp[self.inner_slice()]
        for axis, apply_along_axis in enumerate(along_axes):
            if apply_along_axis:
                shift = np.zeros(3, dtype=int)
                shift[axis] = 1
                out[self.inner_slice()] -= inp[self.inner_slice(shift)]
                out[self.inner_slice()] -= inp[self.inner_slice(-shift)]
