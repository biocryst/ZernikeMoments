/*

                          3D Zernike Moments
    Copyright (C) 2003 by Computer Graphics Group, University of Bonn
           http://www.cg.cs.uni-bonn.de/project-pages/3dsearch/

Code by Marcin Novotni:     marcin@cs.uni-bonn.de

for more information, see the paper:

@inproceedings{novotni-2003-3d,
    author = {M. Novotni and R. Klein},
    title = {3{D} {Z}ernike Descriptors for Content Based Shape Retrieval},
    booktitle = {The 8th ACM Symposium on Solid Modeling and Applications},
    pages = {216--225},
    year = {2003},
    month = {June},
    institution = {Universit\"{a}t Bonn},
    conference = {The 8th ACM Symposium on Solid Modeling and Applications, June 16-20, Seattle, WA}
}
 *---------------------------------------------------------------------------*
 *                                                                           *
 *                                License                                    *
 *                                                                           *
 *  This library is free software; you can redistribute it and/or modify it  *
 *  under the terms of the GNU Library General Public License as published   *
 *  by the Free Software Foundation, version 2.                              *
 *                                                                           *
 *  This library is distributed in the hope that it will be useful, but      *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of               *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *  Library General Public License for more details.                         *
 *                                                                           *
 *  You should have received a copy of the GNU Library General Public        *
 *  License along with this library; if not, write to the Free Software      *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *                                                                           *
\*===========================================================================*/

#include <fstream>
#include "ZernikeDescriptor.h"

template<class T, class InputVoxelIterator>
ZernikeDescriptor<T, InputVoxelIterator>::ZernikeDescriptor(InputVoxelIterator voxels, int _dim, int _order) :
    dim_(_dim), order_(_order)
{
    ComputeNormalization(voxels);
    NormalizeGrid(voxels);

    ComputeMoments(voxels);
    ComputeInvariants();
}

template<class T, class InputVoxelIterator>
void ZernikeDescriptor<T, InputVoxelIterator>::ComputeMoments(InputVoxelIterator voxels)
{
    gm_.Init(voxels, dim_, dim_, dim_, xCOG_, yCOG_, zCOG_, scale_, order_);

    // Zernike moments
    zm_.Init(order_, gm_);
    zm_.Compute();
}

/**
 * Cuts off the function : the object is mapped into the unit ball according to
 * the precomputed center of gravity and scaling factor. All the voxels remaining
 * outside the unit ball are set to zero.
 */
template<class T, class InputVoxelIterator>
void ZernikeDescriptor<T, InputVoxelIterator>::NormalizeGrid(InputVoxelIterator voxels)
{
    T point[3];

    // it is easier to work with squared radius -> no sqrt required
    T radius = (T)1 / scale_;
    T sqrRadius = radius * radius;

    for (size_t x = 0; x < dim_; ++x)
    {
        for (size_t y = 0; y < dim_; ++y)
        {
            for (size_t z = 0; z < dim_; ++z)
            {
                size_t index{ (z * dim_ + y) * dim_ + x };

                if (voxels[index] != (T)0)
                {
                    point[0] = (T)x - xCOG_;
                    point[1] = (T)y - yCOG_;
                    point[2] = (T)z - zCOG_;

                    T sqrLen = point[0] * point[0] + point[1] * point[1] + point[2] * point[2];
                    if (sqrLen > sqrRadius)
                    {
                        voxels[index] = 0.0;
                    }
                }
            }
        }
    }
}

/**
 * Center of gravity and a scaling factor is computed according to the geometrical
 * moments and a bounding sphere around the cog.
 */
template<class T, class InputVoxelIterator>
void ZernikeDescriptor<T, InputVoxelIterator>::ComputeNormalization(InputVoxelIterator voxels)
{
    ScaledGeometricalMoments<InputVoxelIterator, T> gm(voxels, dim_, dim_, dim_, 0.0, 0.0, 0.0, 1.0);

    // compute the geometrical transform for no translation and scaling, first
    // to get the 0'th and 1'st order properties of the function
    //gm.Compute ();

    // 0'th order moments -> normalization
    // 1'st order moments -> center of gravity
    zeroMoment_ = gm.GetMoment(0, 0, 0);
    xCOG_ = gm.GetMoment(1, 0, 0) / zeroMoment_;
    yCOG_ = gm.GetMoment(0, 1, 0) / zeroMoment_;
    zCOG_ = gm.GetMoment(0, 0, 1) / zeroMoment_;

    // scaling, so that the function gets mapped into the unit sphere

    //T recScale = ComputeScale_BoundingSphere (voxels_, dim_, xCOG_, yCOG_, zCOG_);
    T recScale = 2.0 * ComputeScale_RadiusVar(voxels, dim_, xCOG_, yCOG_, zCOG_);

    if (recScale == 0.0)
    {
        std::cerr << "\nNo voxels in grid!\n";
        exit(-1);
    }
    scale_ = (T)1 / recScale;
}

/**
 * Computes the bigest distance from the given COG to any voxel with value bigger than 0.9
 * I.e. I think a binary volume is implicitly assumed here.
 */
template<class T, class InputVoxelIterator>
double ZernikeDescriptor<T, InputVoxelIterator>::ComputeScale_BoundingSphere(InputVoxelIterator voxels, int _dim, T _xCOG, T _yCOG, T _zCOG)
{
    T max = (T)0;

    // the edge length of the voxel grid in voxel units
    int d = _dim;

    for (int x = 0; x < d; ++x)
    {
        for (int y = 0; y < d; ++y)
        {
            for (int z = 0; z < d; ++z)
            {
                size_t index{ (z + d * y) * d + x };

                if (voxels[index] > 0.9)
                {
                    T mx = (T)x - _xCOG;
                    T my = (T)y - _yCOG;
                    T mz = (T)z - _zCOG;
                    T temp = mx * mx + my * my + mz * mz;

                    if (temp > max)
                    {
                        max = temp;
                    }
                }
            }
        }
    }

    return std::sqrt(max);
}

/**
 * Computes the average distance from the given COG to all voxels with value bigger than 0.9
 * I.e. I think a binary volume is implicitly assumed here.
 */
template<class T, class InputVoxelIterator>
double ZernikeDescriptor<T, InputVoxelIterator>::ComputeScale_RadiusVar(InputVoxelIterator _voxels, int _dim, T _xCOG, T _yCOG, T _zCOG)
{
    // the edge length of the voxel grid in voxel units
    int d = _dim;

    int nVoxels = 0;

    T sum = 0.0;

    for (int x = 0; x < d; ++x)
    {
        for (int y = 0; y < d; ++y)
        {
            for (int z = 0; z < d; ++z)
            {
                if (_voxels[(z + d * y) * d + x] > 0.9)
                {
                    T mx = (T)x - _xCOG;
                    T my = (T)y - _yCOG;
                    T mz = (T)z - _zCOG;
                    T temp = mx * mx + my * my + mz * mz;

                    sum += temp;

                    nVoxels++;
                }
            }
        }
    }

    T retval = sqrt(sum / nVoxels);

    return retval;
}

template<class T, class InputVoxelIterator>
void ZernikeDescriptor<T, InputVoxelIterator>::Reconstruct(ComplexT3D& _grid, int _minN, int _maxN, int _minL, int _maxL)
{
    // the scaling between the reconstruction and original grid
    T fac = (T)(_grid.size()) / (T)dim_;

    zm_.Reconstruct(_grid,         // result grid
        xCOG_ * fac,     // center of gravity properly scaled
        yCOG_ * fac,
        zCOG_ * fac,
        scale_ / fac,    // scaling factor
        _minN, _maxN,  // min and max freq. components to be reconstructed
        _minL, _maxL);
}

/**
 * Computes the Zernike moment based invariants, i.e. the norms of vectors with
 * components of Z_nl^m with m being the running index.
 */
template<class T, class InputVoxelIterator>
void ZernikeDescriptor<T, InputVoxelIterator>::ComputeInvariants()
{
    //invariants_.resize (order_ + 1);
    invariants_.clear();
    for (int n = 0; n < order_ + 1; ++n)
    {
        //invariants_[n].resize (n/2 + 1);

        T sum = (T)0;
        int l0 = n % 2, li = 0;

        for (int l = n % 2; l <= n; ++li, l += 2)
        {
            for (int m = -l; m <= l; ++m)
            {
                ComplexT moment = zm_.GetMoment(n, l, m);
                sum += std::norm(moment);
            }

            invariants_.push_back(sqrt(sum));
            //invariants_[n][li] = std::sqrt (sum);
        }
    }
}

template<class T, class InputVoxelIterator>
bool ZernikeDescriptor<T, InputVoxelIterator>::SaveInvariants(const std::string& path_to_file)
{
    std::ofstream outfile(path_to_file, std::ios_base::out);

    if (!outfile.is_open())
    {
        std::cerr << "Cannot open " << path_to_file << std::endl;
        return false;
    }

    std::size_t dim = invariants_.size();

    outfile << dim << ' ';

    if (!outfile.good())
    {
        std::cerr << "Unexpected IO error. Cannot write to " << path_to_file << std::endl;
        return false;
    }

    for (size_t i{ 0 }; i < dim; ++i)
    {
        outfile << invariants_[i] << ' ';

        if (!outfile.good())
        {
            std::cerr << "Unexpected IO error with. Cannot write to " << path_to_file << std::endl;
            return false;
        }
    }

    return true;
}