#pragma once

namespace binvox
{
    namespace utils
    {
        // Change default order of binvox y z x -> x y z
        template<typename VoxelIterator>
        void convert_to_canonical_order(VoxelIterator input, VoxelIterator output, size_t dim)
        {
            for (size_t x = 0; x < dim; x++)
            {
                for (size_t z = 0; z < dim; z++)
                {
                    for (size_t y = 0; y < dim; y++)
                    {
                        output[(z * dim + y) * dim + x] = input[(x * dim + z) * dim + y];
                    }
                }
            }
        }
    }
}