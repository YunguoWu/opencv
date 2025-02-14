/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2014-2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"
#include "opencl_kernels_imgproc.hpp"
#include "opencv2/core/hal/intrin.hpp"

#include "opencv2/core/openvx/ovx_defs.hpp"

namespace cv
{

template<typename T, int shift> struct FixPtCast
{
    typedef int type1;
    typedef T rtype;
    rtype operator ()(type1 arg) const { return (T)((arg + (1 << (shift-1))) >> shift); }
};

template<typename T, int shift> struct FltCast
{
    typedef T type1;
    typedef T rtype;
    rtype operator ()(type1 arg) const { return arg*(T)(1./(1 << shift)); }
};

template<typename T1, typename T2> struct PyrDownNoVec
{
    int operator()(T1**, T2*, int, int) const { return 0; }
};

template<typename T1, typename T2> struct PyrUpNoVec
{
    int operator()(T1**, T2**, int, int) const { return 0; }
};

#if CV_SIMD

struct PyrDownVec_32s8u
{
    int operator()(int** src, uchar* dst, int, int width) const
    {
        int x = 0;
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2], *row3 = src[3], *row4 = src[4];

        for( ; x <= width - v_uint8::nlanes; x += v_uint8::nlanes )
        {
            v_uint16 r0, r1, r2, r3, r4, t0, t1;
            r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)));
            r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)));
            r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)));
            r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x), vx_load(row3 + x + v_int32::nlanes)));
            r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x), vx_load(row4 + x + v_int32::nlanes)));
            t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
            r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x + 2*v_int32::nlanes), vx_load(row0 + x + 3*v_int32::nlanes)));
            r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x + 2*v_int32::nlanes), vx_load(row1 + x + 3*v_int32::nlanes)));
            r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x + 2*v_int32::nlanes), vx_load(row2 + x + 3*v_int32::nlanes)));
            r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x + 2*v_int32::nlanes), vx_load(row3 + x + 3*v_int32::nlanes)));
            r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x + 2*v_int32::nlanes), vx_load(row4 + x + 3*v_int32::nlanes)));
            t1 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
            v_store(dst + x, v_rshr_pack<8>(t0, t1));
        }
        if (x <= width - v_int16::nlanes)
        {
            v_uint16 r0, r1, r2, r3, r4, t0;
            r0 = v_reinterpret_as_u16(v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)));
            r1 = v_reinterpret_as_u16(v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)));
            r2 = v_reinterpret_as_u16(v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)));
            r3 = v_reinterpret_as_u16(v_pack(vx_load(row3 + x), vx_load(row3 + x + v_int32::nlanes)));
            r4 = v_reinterpret_as_u16(v_pack(vx_load(row4 + x), vx_load(row4 + x + v_int32::nlanes)));
            t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);
            v_rshr_pack_store<8>(dst + x, t0);
            x += v_uint16::nlanes;
        }
        for ( ; x <= width - v_int32x4::nlanes; x += v_int32x4::nlanes)
        {
            v_int32x4 r0, r1, r2, r3, r4, t0;
            r0 = v_load(row0 + x);
            r1 = v_load(row1 + x);
            r2 = v_load(row2 + x);
            r3 = v_load(row3 + x);
            r4 = v_load(row4 + x);
            t0 = r0 + r4 + (r2 + r2) + ((r1 + r3 + r2) << 2);

            *(int*)(dst + x) = v_reinterpret_as_s32(v_rshr_pack<8>(v_pack_u(t0, t0), v_setzero_u16())).get0();
        }

        return x;
    }
};

struct PyrDownVec_32f
{
    int operator()(float** src, float* dst, int, int width) const
    {
        int x = 0;
        const float *row0 = src[0], *row1 = src[1], *row2 = src[2], *row3 = src[3], *row4 = src[4];

        v_float32 _4 = vx_setall_f32(4.f), _scale = vx_setall_f32(1.f/256);
        for( ; x <= width - v_float32::nlanes; x += v_float32::nlanes)
        {
            v_float32 r0, r1, r2, r3, r4;
            r0 = vx_load(row0 + x);
            r1 = vx_load(row1 + x);
            r2 = vx_load(row2 + x);
            r3 = vx_load(row3 + x);
            r4 = vx_load(row4 + x);
            v_store(dst + x, v_muladd(r1 + r3 + r2, _4, r0 + r4 + (r2 + r2)) * _scale);
        }

        return x;
    }
};

#if CV_SSE4_1 || CV_NEON || CV_MSA || CV_VSX

struct PyrDownVec_32s16u
{
    int operator()(int** src, ushort* dst, int, int width) const
    {
        int x = 0;
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2], *row3 = src[3], *row4 = src[4];

        for( ; x <= width - v_uint16::nlanes; x += v_uint16::nlanes)
        {
            v_int32 r00 = vx_load(row0 + x),
                    r01 = vx_load(row0 + x + v_int32::nlanes),
                    r10 = vx_load(row1 + x),
                    r11 = vx_load(row1 + x + v_int32::nlanes),
                    r20 = vx_load(row2 + x),
                    r21 = vx_load(row2 + x + v_int32::nlanes),
                    r30 = vx_load(row3 + x),
                    r31 = vx_load(row3 + x + v_int32::nlanes),
                    r40 = vx_load(row4 + x),
                    r41 = vx_load(row4 + x + v_int32::nlanes);
            v_store(dst + x, v_rshr_pack_u<8>(r00 + r40 + (r20 + r20) + ((r10 + r20 + r30) << 2),
                                              r01 + r41 + (r21 + r21) + ((r11 + r21 + r31) << 2)));
        }
        if (x <= width - v_int32::nlanes)
        {
            v_int32 r00 = vx_load(row0 + x),
                    r10 = vx_load(row1 + x),
                    r20 = vx_load(row2 + x),
                    r30 = vx_load(row3 + x),
                    r40 = vx_load(row4 + x);
            v_rshr_pack_u_store<8>(dst + x, r00 + r40 + (r20 + r20) + ((r10 + r20 + r30) << 2));
            x += v_int32::nlanes;
        }

        return x;
    }
};

#else

typedef PyrDownNoVec<int, ushort> PyrDownVec_32s16u;

#endif

struct PyrDownVec_32s16s
{
    int operator()(int** src, short* dst, int, int width) const
    {
        int x = 0;
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2], *row3 = src[3], *row4 = src[4];

        for( ; x <= width - v_int16::nlanes; x += v_int16::nlanes)
        {
            v_int32 r00 = vx_load(row0 + x),
                    r01 = vx_load(row0 + x + v_int32::nlanes),
                    r10 = vx_load(row1 + x),
                    r11 = vx_load(row1 + x + v_int32::nlanes),
                    r20 = vx_load(row2 + x),
                    r21 = vx_load(row2 + x + v_int32::nlanes),
                    r30 = vx_load(row3 + x),
                    r31 = vx_load(row3 + x + v_int32::nlanes),
                    r40 = vx_load(row4 + x),
                    r41 = vx_load(row4 + x + v_int32::nlanes);
            v_store(dst + x, v_rshr_pack<8>(r00 + r40 + (r20 + r20) + ((r10 + r20 + r30) << 2),
                                            r01 + r41 + (r21 + r21) + ((r11 + r21 + r31) << 2)));
        }
        if (x <= width - v_int32::nlanes)
        {
            v_int32 r00 = vx_load(row0 + x),
                r10 = vx_load(row1 + x),
                r20 = vx_load(row2 + x),
                r30 = vx_load(row3 + x),
                r40 = vx_load(row4 + x);
            v_rshr_pack_store<8>(dst + x, r00 + r40 + (r20 + r20) + ((r10 + r20 + r30) << 2));
            x += v_int32::nlanes;
        }

        return x;
    }
};

struct PyrUpVec_32s8u
{
    int operator()(int** src, uchar** dst, int, int width) const
    {
        int x = 0;
        uchar *dst0 = dst[0], *dst1 = dst[1];
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2];

        for( ; x <= width - v_uint8::nlanes; x += v_uint8::nlanes)
        {
            v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
                    v_r01 = v_pack(vx_load(row0 + x + 2 * v_int32::nlanes), vx_load(row0 + x + 3 * v_int32::nlanes)),
                    v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
                    v_r11 = v_pack(vx_load(row1 + x + 2 * v_int32::nlanes), vx_load(row1 + x + 3 * v_int32::nlanes)),
                    v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes)),
                    v_r21 = v_pack(vx_load(row2 + x + 2 * v_int32::nlanes), vx_load(row2 + x + 3 * v_int32::nlanes));
            v_int16 v_2r10 = v_r10 + v_r10, v_2r11 = (v_r11 + v_r11);
            v_store(dst0 + x, v_rshr_pack_u<6>(v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), v_r01 + v_r21 + (v_2r11 + v_2r11 + v_2r11)));
            v_store(dst1 + x, v_rshr_pack_u<6>((v_r10 + v_r20) << 2, (v_r11 + v_r21) << 2));
        }
        if(x <= width - v_uint16::nlanes)
        {
            v_int16 v_r00 = v_pack(vx_load(row0 + x), vx_load(row0 + x + v_int32::nlanes)),
                    v_r10 = v_pack(vx_load(row1 + x), vx_load(row1 + x + v_int32::nlanes)),
                    v_r20 = v_pack(vx_load(row2 + x), vx_load(row2 + x + v_int32::nlanes));
            v_int16 v_2r10 = v_r10 + v_r10;
            v_rshr_pack_u_store<6>(dst0 + x, v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10));
            v_rshr_pack_u_store<6>(dst1 + x, (v_r10 + v_r20) << 2);
            x += v_uint16::nlanes;
        }
        for (; x <= width - v_int32x4::nlanes; x += v_int32x4::nlanes)
        {
            v_int32 v_r00 = vx_load(row0 + x),
                    v_r10 = vx_load(row1 + x),
                    v_r20 = vx_load(row2 + x);
            v_int32 v_2r10 = v_r10 + v_r10;
            v_int16 d = v_pack(v_r00 + v_r20 + (v_2r10 + v_2r10 + v_2r10), (v_r10 + v_r20) << 2);
            *(int*)(dst0 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(d, vx_setzero_s16())).get0();
            *(int*)(dst1 + x) = v_reinterpret_as_s32(v_rshr_pack_u<6>(v_combine_high(d, d), vx_setzero_s16())).get0();
        }

        return x;
    }
};

struct PyrUpVec_32s16s
{
    int operator()(int** src, short** dst, int, int width) const
    {
        int x = 0;
        short *dst0 = dst[0], *dst1 = dst[1];
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2];

        for( ; x <= width - v_int16::nlanes; x += v_int16::nlanes)
        {
            v_int32 v_r00 = vx_load(row0 + x),
                    v_r01 = vx_load(row0 + x + v_int32::nlanes),
                    v_r10 = vx_load(row1 + x),
                    v_r11 = vx_load(row1 + x + v_int32::nlanes),
                    v_r20 = vx_load(row2 + x),
                    v_r21 = vx_load(row2 + x + v_int32::nlanes);
            v_store(dst0 + x, v_rshr_pack<6>(v_r00 + v_r20 + ((v_r10 << 1) + (v_r10 << 2)), v_r01 + v_r21 + ((v_r11 << 1) + (v_r11 << 2))));
            v_store(dst1 + x, v_rshr_pack<6>((v_r10 + v_r20) << 2, (v_r11 + v_r21) << 2));
        }
        if(x <= width - v_int32::nlanes)
        {
            v_int32 v_r00 = vx_load(row0 + x),
                    v_r10 = vx_load(row1 + x),
                    v_r20 = vx_load(row2 + x);
            v_rshr_pack_store<6>(dst0 + x, v_r00 + v_r20 + ((v_r10 << 1) + (v_r10 << 2)));
            v_rshr_pack_store<6>(dst1 + x, (v_r10 + v_r20) << 2);
            x += v_int32::nlanes;
        }

        return x;
    }
};

#if CV_SSE4_1 || CV_NEON || CV_MSA || CV_VSX

struct PyrUpVec_32s16u
{
    int operator()(int** src, ushort** dst, int, int width) const
    {
        int x = 0;
        ushort *dst0 = dst[0], *dst1 = dst[1];
        const int *row0 = src[0], *row1 = src[1], *row2 = src[2];

        for( ; x <= width - v_uint16::nlanes; x += v_uint16::nlanes)
        {
            v_int32 v_r00 = vx_load(row0 + x),
                    v_r01 = vx_load(row0 + x + v_int32::nlanes),
                    v_r10 = vx_load(row1 + x),
                    v_r11 = vx_load(row1 + x + v_int32::nlanes),
                    v_r20 = vx_load(row2 + x),
                    v_r21 = vx_load(row2 + x + v_int32::nlanes);
            v_store(dst0 + x, v_rshr_pack_u<6>(v_r00 + v_r20 + ((v_r10 << 1) + (v_r10 << 2)), v_r01 + v_r21 + ((v_r11 << 1) + (v_r11 << 2))));
            v_store(dst1 + x, v_rshr_pack_u<6>((v_r10 + v_r20) << 2, (v_r11 + v_r21) << 2));
        }
        if(x <= width - v_int32::nlanes)
        {
            v_int32 v_r00 = vx_load(row0 + x),
                    v_r10 = vx_load(row1 + x),
                    v_r20 = vx_load(row2 + x);
            v_rshr_pack_u_store<6>(dst0 + x, v_r00 + v_r20 + ((v_r10 << 1) + (v_r10 << 2)));
            v_rshr_pack_u_store<6>(dst1 + x, (v_r10 + v_r20) << 2);
            x += v_int32::nlanes;
        }

        return x;
    }
};

#else

typedef PyrUpNoVec<int, ushort> PyrUpVec_32s16u;

#endif // CV_SSE4_1

struct PyrUpVec_32f
{
    int operator()(float** src, float** dst, int, int width) const
    {
        int x = 0;
        const float *row0 = src[0], *row1 = src[1], *row2 = src[2];
        float *dst0 = dst[0], *dst1 = dst[1];

        v_float32 v_6 = vx_setall_f32(6.0f), v_scale = vx_setall_f32(1.f/64.f), v_scale4 = vx_setall_f32(1.f/16.f);
        for( ; x <= width - v_float32::nlanes; x += v_float32::nlanes)
        {
            v_float32 v_r0 = vx_load(row0 + x),
                      v_r1 = vx_load(row1 + x),
                      v_r2 = vx_load(row2 + x);
            v_store(dst1 + x, v_scale4 * (v_r1 + v_r2));
            v_store(dst0 + x, v_scale * (v_muladd(v_6, v_r1, v_r0) + v_r2));
        }

        return x;
    }
};

#else

typedef PyrDownNoVec<int, uchar> PyrDownVec_32s8u;
typedef PyrDownNoVec<int, ushort> PyrDownVec_32s16u;
typedef PyrDownNoVec<int, short> PyrDownVec_32s16s;
typedef PyrDownNoVec<float, float> PyrDownVec_32f;

typedef PyrUpNoVec<int, uchar> PyrUpVec_32s8u;
typedef PyrUpNoVec<int, short> PyrUpVec_32s16s;
typedef PyrUpNoVec<int, ushort> PyrUpVec_32s16u;
typedef PyrUpNoVec<float, float> PyrUpVec_32f;

#endif

template<class CastOp, class VecOp> void
pyrDown_( const Mat& _src, Mat& _dst, int borderType )
{
    const int PD_SZ = 5;
    typedef typename CastOp::type1 WT;
    typedef typename CastOp::rtype T;

    CV_Assert( !_src.empty() );
    Size ssize = _src.size(), dsize = _dst.size();
    int cn = _src.channels();
    int bufstep = (int)alignSize(dsize.width*cn, 16);
    AutoBuffer<WT> _buf(bufstep*PD_SZ + 16);
    WT* buf = alignPtr((WT*)_buf.data(), 16);
    int tabL[CV_CN_MAX*(PD_SZ+2)], tabR[CV_CN_MAX*(PD_SZ+2)];
    AutoBuffer<int> _tabM(dsize.width*cn);
    int* tabM = _tabM.data();
    WT* rows[PD_SZ];
    CastOp castOp;
    VecOp vecOp;

    CV_Assert( ssize.width > 0 && ssize.height > 0 &&
               std::abs(dsize.width*2 - ssize.width) <= 2 &&
               std::abs(dsize.height*2 - ssize.height) <= 2 );
    int k, x, sy0 = -PD_SZ/2, sy = sy0, width0 = std::min((ssize.width-PD_SZ/2-1)/2 + 1, dsize.width);

    for( x = 0; x <= PD_SZ+1; x++ )
    {
        int sx0 = borderInterpolate(x - PD_SZ/2, ssize.width, borderType)*cn;
        int sx1 = borderInterpolate(x + width0*2 - PD_SZ/2, ssize.width, borderType)*cn;
        for( k = 0; k < cn; k++ )
        {
            tabL[x*cn + k] = sx0 + k;
            tabR[x*cn + k] = sx1 + k;
        }
    }

    ssize.width *= cn;
    dsize.width *= cn;
    width0 *= cn;

    for( x = 0; x < dsize.width; x++ )
        tabM[x] = (x/cn)*2*cn + x % cn;

    for( int y = 0; y < dsize.height; y++ )
    {
        T* dst = _dst.ptr<T>(y);
        WT *row0, *row1, *row2, *row3, *row4;

        // fill the ring buffer (horizontal convolution and decimation)
        for( ; sy <= y*2 + 2; sy++ )
        {
            WT* row = buf + ((sy - sy0) % PD_SZ)*bufstep;
            int _sy = borderInterpolate(sy, ssize.height, borderType);
            const T* src = _src.ptr<T>(_sy);
            int limit = cn;
            const int* tab = tabL;

            for( x = 0;;)
            {
                for( ; x < limit; x++ )
                {
                    row[x] = src[tab[x+cn*2]]*6 + (src[tab[x+cn]] + src[tab[x+cn*3]])*4 +
                        src[tab[x]] + src[tab[x+cn*4]];
                }

                if( x == dsize.width )
                    break;

                if( cn == 1 )
                {
                    for( ; x < width0; x++ )
                        row[x] = src[x*2]*6 + (src[x*2 - 1] + src[x*2 + 1])*4 +
                            src[x*2 - 2] + src[x*2 + 2];
                }
                else if( cn == 3 )
                {
                    for( ; x < width0; x += 3 )
                    {
                        const T* s = src + x*2;
                        WT t0 = s[0]*6 + (s[-3] + s[3])*4 + s[-6] + s[6];
                        WT t1 = s[1]*6 + (s[-2] + s[4])*4 + s[-5] + s[7];
                        WT t2 = s[2]*6 + (s[-1] + s[5])*4 + s[-4] + s[8];
                        row[x] = t0; row[x+1] = t1; row[x+2] = t2;
                    }
                }
                else if( cn == 4 )
                {
                    for( ; x < width0; x += 4 )
                    {
                        const T* s = src + x*2;
                        WT t0 = s[0]*6 + (s[-4] + s[4])*4 + s[-8] + s[8];
                        WT t1 = s[1]*6 + (s[-3] + s[5])*4 + s[-7] + s[9];
                        row[x] = t0; row[x+1] = t1;
                        t0 = s[2]*6 + (s[-2] + s[6])*4 + s[-6] + s[10];
                        t1 = s[3]*6 + (s[-1] + s[7])*4 + s[-5] + s[11];
                        row[x+2] = t0; row[x+3] = t1;
                    }
                }
                else
                {
                    for( ; x < width0; x++ )
                    {
                        int sx = tabM[x];
                        row[x] = src[sx]*6 + (src[sx - cn] + src[sx + cn])*4 +
                            src[sx - cn*2] + src[sx + cn*2];
                    }
                }

                limit = dsize.width;
                tab = tabR - x;
            }
        }

        // do vertical convolution and decimation and write the result to the destination image
        for( k = 0; k < PD_SZ; k++ )
            rows[k] = buf + ((y*2 - PD_SZ/2 + k - sy0) % PD_SZ)*bufstep;
        row0 = rows[0]; row1 = rows[1]; row2 = rows[2]; row3 = rows[3]; row4 = rows[4];

        x = vecOp(rows, dst, (int)_dst.step, dsize.width);
        for( ; x < dsize.width; x++ )
            dst[x] = castOp(row2[x]*6 + (row1[x] + row3[x])*4 + row0[x] + row4[x]);
    }
}


template<class CastOp, class VecOp> void
pyrUp_( const Mat& _src, Mat& _dst, int)
{
    const int PU_SZ = 3;
    typedef typename CastOp::type1 WT;
    typedef typename CastOp::rtype T;

    Size ssize = _src.size(), dsize = _dst.size();
    int cn = _src.channels();
    int bufstep = (int)alignSize((dsize.width+1)*cn, 16);
    AutoBuffer<WT> _buf(bufstep*PU_SZ + 16);
    WT* buf = alignPtr((WT*)_buf.data(), 16);
    AutoBuffer<int> _dtab(ssize.width*cn);
    int* dtab = _dtab.data();
    WT* rows[PU_SZ];
    T* dsts[2];
    CastOp castOp;
    VecOp vecOp;

    CV_Assert( std::abs(dsize.width - ssize.width*2) == dsize.width % 2 &&
               std::abs(dsize.height - ssize.height*2) == dsize.height % 2);
    int k, x, sy0 = -PU_SZ/2, sy = sy0;

    ssize.width *= cn;
    dsize.width *= cn;

    for( x = 0; x < ssize.width; x++ )
        dtab[x] = (x/cn)*2*cn + x % cn;

    for( int y = 0; y < ssize.height; y++ )
    {
        T* dst0 = _dst.ptr<T>(y*2);
        T* dst1 = _dst.ptr<T>(std::min(y*2+1, dsize.height-1));
        WT *row0, *row1, *row2;

        // fill the ring buffer (horizontal convolution and decimation)
        for( ; sy <= y + 1; sy++ )
        {
            WT* row = buf + ((sy - sy0) % PU_SZ)*bufstep;
            int _sy = borderInterpolate(sy*2, ssize.height*2, BORDER_REFLECT_101)/2;
            const T* src = _src.ptr<T>(_sy);

            if( ssize.width == cn )
            {
                for( x = 0; x < cn; x++ )
                    row[x] = row[x + cn] = src[x]*8;
                continue;
            }

            for( x = 0; x < cn; x++ )
            {
                int dx = dtab[x];
                WT t0 = src[x]*6 + src[x + cn]*2;
                WT t1 = (src[x] + src[x + cn])*4;
                row[dx] = t0; row[dx + cn] = t1;
                dx = dtab[ssize.width - cn + x];
                int sx = ssize.width - cn + x;
                t0 = src[sx - cn] + src[sx]*7;
                t1 = src[sx]*8;
                row[dx] = t0; row[dx + cn] = t1;

                if (dsize.width > ssize.width*2)
                {
                    row[(_dst.cols-1) + x] = row[dx + cn];
                }
            }

            for( x = cn; x < ssize.width - cn; x++ )
            {
                int dx = dtab[x];
                WT t0 = src[x-cn] + src[x]*6 + src[x+cn];
                WT t1 = (src[x] + src[x+cn])*4;
                row[dx] = t0;
                row[dx+cn] = t1;
            }
        }

        // do vertical convolution and decimation and write the result to the destination image
        for( k = 0; k < PU_SZ; k++ )
            rows[k] = buf + ((y - PU_SZ/2 + k - sy0) % PU_SZ)*bufstep;
        row0 = rows[0]; row1 = rows[1]; row2 = rows[2];
        dsts[0] = dst0; dsts[1] = dst1;

        x = vecOp(rows, dsts, (int)_dst.step, dsize.width);
        for( ; x < dsize.width; x++ )
        {
            T t1 = castOp((row1[x] + row2[x])*4);
            T t0 = castOp(row0[x] + row1[x]*6 + row2[x]);
            dst1[x] = t1; dst0[x] = t0;
        }
    }

    if (dsize.height > ssize.height*2)
    {
        T* dst0 = _dst.ptr<T>(ssize.height*2-2);
        T* dst2 = _dst.ptr<T>(ssize.height*2);

        for(x = 0; x < dsize.width ; x++ )
        {
            dst2[x] = dst0[x];
        }
    }
}

typedef void (*PyrFunc)(const Mat&, Mat&, int);

#ifdef HAVE_OPENCL

static bool ocl_pyrDown( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType)
{
    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);

    bool doubleSupport = ocl::Device::getDefault().doubleFPConfig() > 0;
    if (cn > 4 || (depth == CV_64F && !doubleSupport))
        return false;

    Size ssize = _src.size();
    Size dsize = _dsz.empty() ? Size((ssize.width + 1) / 2, (ssize.height + 1) / 2) : _dsz;
    if (dsize.height < 2 || dsize.width < 2)
        return false;

    CV_Assert( ssize.width > 0 && ssize.height > 0 &&
            std::abs(dsize.width*2 - ssize.width) <= 2 &&
            std::abs(dsize.height*2 - ssize.height) <= 2 );

    UMat src = _src.getUMat();
    _dst.create( dsize, src.type() );
    UMat dst = _dst.getUMat();

    int float_depth = depth == CV_64F ? CV_64F : CV_32F;
    const int local_size = 256;
    int kercn = 1;
    if (depth == CV_8U && float_depth == CV_32F && cn == 1 && ocl::Device::getDefault().isIntel())
        kercn = 4;
    const char * const borderMap[] = { "BORDER_CONSTANT", "BORDER_REPLICATE", "BORDER_REFLECT", "BORDER_WRAP",
                                       "BORDER_REFLECT_101" };
    char cvt[2][50];
    String buildOptions = format(
            "-D T=%s -D FT=%s -D convertToT=%s -D convertToFT=%s%s "
            "-D T1=%s -D cn=%d -D kercn=%d -D fdepth=%d -D %s -D LOCAL_SIZE=%d",
            ocl::typeToStr(type), ocl::typeToStr(CV_MAKETYPE(float_depth, cn)),
            ocl::convertTypeStr(float_depth, depth, cn, cvt[0]),
            ocl::convertTypeStr(depth, float_depth, cn, cvt[1]),
            doubleSupport ? " -D DOUBLE_SUPPORT" : "", ocl::typeToStr(depth),
            cn, kercn, float_depth, borderMap[borderType], local_size
    );
    ocl::Kernel k("pyrDown", ocl::imgproc::pyr_down_oclsrc, buildOptions);
    if (k.empty())
        return false;

    k.args(ocl::KernelArg::ReadOnly(src), ocl::KernelArg::WriteOnly(dst));

    size_t localThreads[2]  = { (size_t)local_size/kercn, 1 };
    size_t globalThreads[2] = { ((size_t)src.cols + (kercn-1))/kercn, ((size_t)dst.rows + 1) / 2 };
    return k.run(2, globalThreads, localThreads, false);
}

static bool ocl_pyrUp( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType)
{
    int type = _src.type(), depth = CV_MAT_DEPTH(type), channels = CV_MAT_CN(type);

    if (channels > 4 || borderType != BORDER_DEFAULT)
        return false;

    bool doubleSupport = ocl::Device::getDefault().doubleFPConfig() > 0;
    if (depth == CV_64F && !doubleSupport)
        return false;

    Size ssize = _src.size();
    if (!_dsz.empty() && (_dsz != Size(ssize.width * 2, ssize.height * 2)))
        return false;

    UMat src = _src.getUMat();
    Size dsize = Size(ssize.width * 2, ssize.height * 2);
    _dst.create( dsize, src.type() );
    UMat dst = _dst.getUMat();

    int float_depth = depth == CV_64F ? CV_64F : CV_32F;
    const int local_size = 16;
    char cvt[2][50];
    String buildOptions = format(
            "-D T=%s -D FT=%s -D convertToT=%s -D convertToFT=%s%s "
            "-D T1=%s -D cn=%d -D LOCAL_SIZE=%d",
            ocl::typeToStr(type), ocl::typeToStr(CV_MAKETYPE(float_depth, channels)),
            ocl::convertTypeStr(float_depth, depth, channels, cvt[0]),
            ocl::convertTypeStr(depth, float_depth, channels, cvt[1]),
            doubleSupport ? " -D DOUBLE_SUPPORT" : "",
            ocl::typeToStr(depth), channels, local_size
    );
    size_t globalThreads[2] = { (size_t)dst.cols, (size_t)dst.rows };
    size_t localThreads[2] = { (size_t)local_size, (size_t)local_size };
    ocl::Kernel k;
    if (ocl::Device::getDefault().isIntel() && channels == 1)
    {
        if (type == CV_8UC1 && src.cols % 2 == 0)
        {
            buildOptions.clear();
            k.create("pyrUp_cols2", ocl::imgproc::pyramid_up_oclsrc, buildOptions);
            globalThreads[0] = dst.cols/4; globalThreads[1] = dst.rows/2;
        }
        else
        {
            k.create("pyrUp_unrolled", ocl::imgproc::pyr_up_oclsrc, buildOptions);
            globalThreads[0] = dst.cols/2; globalThreads[1] = dst.rows/2;
        }
    }
    else
        k.create("pyrUp", ocl::imgproc::pyr_up_oclsrc, buildOptions);

    if (k.empty())
        return false;

    k.args(ocl::KernelArg::ReadOnly(src), ocl::KernelArg::WriteOnly(dst));
    return k.run(2, globalThreads, localThreads, false);
}

#endif

}

#if defined(HAVE_IPP)
namespace cv
{
static bool ipp_pyrdown( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType )
{
    CV_INSTRUMENT_REGION_IPP();

#if IPP_VERSION_X100 >= 810 && !IPP_DISABLE_PYRAMIDS_DOWN
    Size dsz = _dsz.empty() ? Size((_src.cols() + 1)/2, (_src.rows() + 1)/2) : _dsz;
    bool isolated = (borderType & BORDER_ISOLATED) != 0;
    int borderTypeNI = borderType & ~BORDER_ISOLATED;

    Mat src = _src.getMat();
    _dst.create( dsz, src.type() );
    Mat dst = _dst.getMat();
    int depth = src.depth();


    {
        bool isolated = (borderType & BORDER_ISOLATED) != 0;
        int borderTypeNI = borderType & ~BORDER_ISOLATED;
        if (borderTypeNI == BORDER_DEFAULT && (!src.isSubmatrix() || isolated) && dsz == Size(src.cols*2, src.rows*2))
        {
            typedef IppStatus (CV_STDCALL * ippiPyrUp)(const void* pSrc, int srcStep, void* pDst, int dstStep, IppiSize srcRoi, Ipp8u* buffer);
            int type = src.type();
            CV_SUPPRESS_DEPRECATED_START
            ippiPyrUp pyrUpFunc = type == CV_8UC1 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_8u_C1R :
                                  type == CV_8UC3 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_8u_C3R :
                                  type == CV_32FC1 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_32f_C1R :
                                  type == CV_32FC3 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_32f_C3R : 0;
            CV_SUPPRESS_DEPRECATED_END

            if (pyrUpFunc)
            {
                int bufferSize;
                IppiSize srcRoi = { src.cols, src.rows };
                IppDataType dataType = depth == CV_8U ? ipp8u : ipp32f;
                CV_SUPPRESS_DEPRECATED_START
                IppStatus ok = ippiPyrUpGetBufSize_Gauss5x5(srcRoi.width, dataType, src.channels(), &bufferSize);
                CV_SUPPRESS_DEPRECATED_END
                if (ok >= 0)
                {
                    Ipp8u* buffer = ippsMalloc_8u_L(bufferSize);
                    ok = pyrUpFunc(src.data, (int) src.step, dst.data, (int) dst.step, srcRoi, buffer);
                    ippsFree(buffer);

                    if (ok >= 0)
                    {
                        CV_IMPL_ADD(CV_IMPL_IPP);
                        return true;
                    }
                }
            }
        }
    }
#else
    CV_UNUSED(_src); CV_UNUSED(_dst); CV_UNUSED(_dsz); CV_UNUSED(borderType);
#endif
    return false;
}
}
#endif

#ifdef HAVE_OPENVX
namespace cv
{
static bool openvx_pyrDown( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType )
{
    using namespace ivx;

    Mat srcMat = _src.getMat();

    if (ovx::skipSmallImages<VX_KERNEL_HALFSCALE_GAUSSIAN>(srcMat.cols, srcMat.rows))
        return false;

    CV_Assert(!srcMat.empty());

    Size ssize = _src.size();
    Size acceptableSize = Size((ssize.width + 1) / 2, (ssize.height + 1) / 2);

    // OpenVX limitations
    if((srcMat.type() != CV_8U) ||
       (borderType != BORDER_REPLICATE) ||
       (_dsz != acceptableSize && !_dsz.empty()))
        return false;

    // The only border mode which is supported by both cv::pyrDown() and OpenVX
    // and produces predictable results
    ivx::border_t borderMode;
    borderMode.mode = VX_BORDER_REPLICATE;

    _dst.create( acceptableSize, srcMat.type() );
    Mat dstMat = _dst.getMat();

    CV_Assert( ssize.width > 0 && ssize.height > 0 &&
            std::abs(acceptableSize.width*2 - ssize.width) <= 2 &&
            std::abs(acceptableSize.height*2 - ssize.height) <= 2 );

    try
    {
        Context context = ovx::getOpenVXContext();
        if(context.vendorID() == VX_ID_KHRONOS)
        {
            // This implementation performs floor-like rounding
            // (OpenCV uses floor(x+0.5)-like rounding)
            // and ignores border mode (and loses 1px size border)
            return false;
        }

        Image srcImg = Image::createFromHandle(context, Image::matTypeToFormat(srcMat.type()),
                                               Image::createAddressing(srcMat), (void*)srcMat.data);
        Image dstImg = Image::createFromHandle(context, Image::matTypeToFormat(dstMat.type()),
                                               Image::createAddressing(dstMat), (void*)dstMat.data);

        ivx::Scalar kernelSize = ivx::Scalar::create<VX_TYPE_INT32>(context, 5);
        Graph graph = Graph::create(context);
        ivx::Node halfNode = ivx::Node::create(graph, VX_KERNEL_HALFSCALE_GAUSSIAN, srcImg, dstImg, kernelSize);
        halfNode.setBorder(borderMode);
        graph.verify();
        graph.process();

#ifdef VX_VERSION_1_1
        //we should take user memory back before release
        //(it's not done automatically according to standard)
        srcImg.swapHandle(); dstImg.swapHandle();
#endif
    }
    catch (const RuntimeError & e)
    {
        VX_DbgThrow(e.what());
    }
    catch (const WrapperError & e)
    {
        VX_DbgThrow(e.what());
    }

    return true;
}

}
#endif

void cv::pyrDown( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType )
{
    CV_INSTRUMENT_REGION();

    CV_Assert(borderType != BORDER_CONSTANT);

    CV_OCL_RUN(_src.dims() <= 2 && _dst.isUMat(),
               ocl_pyrDown(_src, _dst, _dsz, borderType))

    CV_OVX_RUN(_src.dims() <= 2,
               openvx_pyrDown(_src, _dst, _dsz, borderType))

    Mat src = _src.getMat();
    Size dsz = _dsz.empty() ? Size((src.cols + 1)/2, (src.rows + 1)/2) : _dsz;
    _dst.create( dsz, src.type() );
    Mat dst = _dst.getMat();
    int depth = src.depth();

    CALL_HAL(pyrDown, cv_hal_pyrdown, src.data, src.step, src.cols, src.rows, dst.data, dst.step, dst.cols, dst.rows, depth, src.channels(), borderType);

#ifdef HAVE_IPP
    bool isolated = (borderType & BORDER_ISOLATED) != 0;
    int borderTypeNI = borderType & ~BORDER_ISOLATED;
#endif
    CV_IPP_RUN(borderTypeNI == BORDER_DEFAULT && (!_src.isSubmatrix() || isolated) && dsz == Size((_src.cols() + 1)/2, (_src.rows() + 1)/2),
        ipp_pyrdown( _src,  _dst,  _dsz,  borderType));


    PyrFunc func = 0;
    if( depth == CV_8U )
        func = pyrDown_<FixPtCast<uchar, 8>, PyrDownVec_32s8u>;
    else if( depth == CV_16S )
        func = pyrDown_<FixPtCast<short, 8>, PyrDownVec_32s16s >;
    else if( depth == CV_16U )
        func = pyrDown_<FixPtCast<ushort, 8>, PyrDownVec_32s16u >;
    else if( depth == CV_32F )
        func = pyrDown_<FltCast<float, 8>, PyrDownVec_32f>;
    else if( depth == CV_64F )
        func = pyrDown_<FltCast<double, 8>, PyrDownNoVec<double, double> >;
    else
        CV_Error( CV_StsUnsupportedFormat, "" );

    func( src, dst, borderType );
}


#if defined(HAVE_IPP)
namespace cv
{
static bool ipp_pyrup( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType )
{
    CV_INSTRUMENT_REGION_IPP();

#if IPP_VERSION_X100 >= 810 && !IPP_DISABLE_PYRAMIDS_UP
    Size sz = _src.dims() <= 2 ? _src.size() : Size();
    Size dsz = _dsz.empty() ? Size(_src.cols()*2, _src.rows()*2) : _dsz;

    Mat src = _src.getMat();
    _dst.create( dsz, src.type() );
    Mat dst = _dst.getMat();
    int depth = src.depth();

    {
        bool isolated = (borderType & BORDER_ISOLATED) != 0;
        int borderTypeNI = borderType & ~BORDER_ISOLATED;
        if (borderTypeNI == BORDER_DEFAULT && (!src.isSubmatrix() || isolated) && dsz == Size(src.cols*2, src.rows*2))
        {
            typedef IppStatus (CV_STDCALL * ippiPyrUp)(const void* pSrc, int srcStep, void* pDst, int dstStep, IppiSize srcRoi, Ipp8u* buffer);
            int type = src.type();
            CV_SUPPRESS_DEPRECATED_START
            ippiPyrUp pyrUpFunc = type == CV_8UC1 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_8u_C1R :
                                  type == CV_8UC3 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_8u_C3R :
                                  type == CV_32FC1 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_32f_C1R :
                                  type == CV_32FC3 ? (ippiPyrUp) ippiPyrUp_Gauss5x5_32f_C3R : 0;
            CV_SUPPRESS_DEPRECATED_END

            if (pyrUpFunc)
            {
                int bufferSize;
                IppiSize srcRoi = { src.cols, src.rows };
                IppDataType dataType = depth == CV_8U ? ipp8u : ipp32f;
                CV_SUPPRESS_DEPRECATED_START
                IppStatus ok = ippiPyrUpGetBufSize_Gauss5x5(srcRoi.width, dataType, src.channels(), &bufferSize);
                CV_SUPPRESS_DEPRECATED_END
                if (ok >= 0)
                {
                    Ipp8u* buffer = ippsMalloc_8u_L(bufferSize);
                    ok = pyrUpFunc(src.data, (int) src.step, dst.data, (int) dst.step, srcRoi, buffer);
                    ippsFree(buffer);

                    if (ok >= 0)
                    {
                        CV_IMPL_ADD(CV_IMPL_IPP);
                        return true;
                    }
                }
            }
        }
    }
#else
    CV_UNUSED(_src); CV_UNUSED(_dst); CV_UNUSED(_dsz); CV_UNUSED(borderType);
#endif
    return false;
}
}
#endif

void cv::pyrUp( InputArray _src, OutputArray _dst, const Size& _dsz, int borderType )
{
    CV_INSTRUMENT_REGION();

    CV_Assert(borderType == BORDER_DEFAULT);

    CV_OCL_RUN(_src.dims() <= 2 && _dst.isUMat(),
               ocl_pyrUp(_src, _dst, _dsz, borderType))


    Mat src = _src.getMat();
    Size dsz = _dsz.empty() ? Size(src.cols*2, src.rows*2) : _dsz;
    _dst.create( dsz, src.type() );
    Mat dst = _dst.getMat();
    int depth = src.depth();

#ifdef HAVE_IPP
    bool isolated = (borderType & BORDER_ISOLATED) != 0;
    int borderTypeNI = borderType & ~BORDER_ISOLATED;
#endif
    CV_IPP_RUN(borderTypeNI == BORDER_DEFAULT && (!_src.isSubmatrix() || isolated) && dsz == Size(_src.cols()*2, _src.rows()*2),
        ipp_pyrup( _src,  _dst,  _dsz,  borderType));


    PyrFunc func = 0;
    if( depth == CV_8U )
        func = pyrUp_<FixPtCast<uchar, 6>, PyrUpVec_32s8u >;
    else if( depth == CV_16S )
        func = pyrUp_<FixPtCast<short, 6>, PyrUpVec_32s16s >;
    else if( depth == CV_16U )
        func = pyrUp_<FixPtCast<ushort, 6>, PyrUpVec_32s16u >;
    else if( depth == CV_32F )
        func = pyrUp_<FltCast<float, 6>, PyrUpVec_32f >;
    else if( depth == CV_64F )
        func = pyrUp_<FltCast<double, 6>, PyrUpNoVec<double, double> >;
    else
        CV_Error( CV_StsUnsupportedFormat, "" );

    func( src, dst, borderType );
}


#ifdef HAVE_IPP
namespace cv
{
static bool ipp_buildpyramid( InputArray _src, OutputArrayOfArrays _dst, int maxlevel, int borderType )
{
    CV_INSTRUMENT_REGION_IPP();

#if IPP_VERSION_X100 >= 810 && !IPP_DISABLE_PYRAMIDS_BUILD
    Mat src = _src.getMat();
    _dst.create( maxlevel + 1, 1, 0 );
    _dst.getMatRef(0) = src;

    int i=1;

    {
        bool isolated = (borderType & BORDER_ISOLATED) != 0;
        int borderTypeNI = borderType & ~BORDER_ISOLATED;
        if (borderTypeNI == BORDER_DEFAULT && (!src.isSubmatrix() || isolated))
        {
            typedef IppStatus (CV_STDCALL * ippiPyramidLayerDownInitAlloc)(void** ppState, IppiSize srcRoi, Ipp32f rate, void* pKernel, int kerSize, int mode);
            typedef IppStatus (CV_STDCALL * ippiPyramidLayerDown)(void* pSrc, int srcStep, IppiSize srcRoiSize, void* pDst, int dstStep, IppiSize dstRoiSize, void* pState);
            typedef IppStatus (CV_STDCALL * ippiPyramidLayerDownFree)(void* pState);

            int type = src.type();
            int depth = src.depth();
            ippiPyramidLayerDownInitAlloc pyrInitAllocFunc = 0;
            ippiPyramidLayerDown pyrDownFunc = 0;
            ippiPyramidLayerDownFree pyrFreeFunc = 0;

            if (type == CV_8UC1)
            {
                pyrInitAllocFunc = (ippiPyramidLayerDownInitAlloc) ippiPyramidLayerDownInitAlloc_8u_C1R;
                pyrDownFunc = (ippiPyramidLayerDown) ippiPyramidLayerDown_8u_C1R;
                pyrFreeFunc = (ippiPyramidLayerDownFree) ippiPyramidLayerDownFree_8u_C1R;
            }
            else if (type == CV_8UC3)
            {
                pyrInitAllocFunc = (ippiPyramidLayerDownInitAlloc) ippiPyramidLayerDownInitAlloc_8u_C3R;
                pyrDownFunc = (ippiPyramidLayerDown) ippiPyramidLayerDown_8u_C3R;
                pyrFreeFunc = (ippiPyramidLayerDownFree) ippiPyramidLayerDownFree_8u_C3R;
            }
            else if (type == CV_32FC1)
            {
                pyrInitAllocFunc = (ippiPyramidLayerDownInitAlloc) ippiPyramidLayerDownInitAlloc_32f_C1R;
                pyrDownFunc = (ippiPyramidLayerDown) ippiPyramidLayerDown_32f_C1R;
                pyrFreeFunc = (ippiPyramidLayerDownFree) ippiPyramidLayerDownFree_32f_C1R;
            }
            else if (type == CV_32FC3)
            {
                pyrInitAllocFunc = (ippiPyramidLayerDownInitAlloc) ippiPyramidLayerDownInitAlloc_32f_C3R;
                pyrDownFunc = (ippiPyramidLayerDown) ippiPyramidLayerDown_32f_C3R;
                pyrFreeFunc = (ippiPyramidLayerDownFree) ippiPyramidLayerDownFree_32f_C3R;
            }

            if (pyrInitAllocFunc && pyrDownFunc && pyrFreeFunc)
            {
                float rate = 2.f;
                IppiSize srcRoi = { src.cols, src.rows };
                IppiPyramid *gPyr;
                IppStatus ok = ippiPyramidInitAlloc(&gPyr, maxlevel + 1, srcRoi, rate);

                Ipp16s iKernel[5] = { 1, 4, 6, 4, 1 };
                Ipp32f fKernel[5] = { 1.f, 4.f, 6.f, 4.f, 1.f };
                void* kernel = depth >= CV_32F ? (void*) fKernel : (void*) iKernel;

                if (ok >= 0) ok = pyrInitAllocFunc((void**) &(gPyr->pState), srcRoi, rate, kernel, 5, IPPI_INTER_LINEAR);
                if (ok >= 0)
                {
                    gPyr->pImage[0] = src.data;
                    gPyr->pStep[0] = (int) src.step;
                    gPyr->pRoi[0] = srcRoi;
                    for( ; i <= maxlevel; i++ )
                    {
                        IppiSize dstRoi;
                        ok = ippiGetPyramidDownROI(gPyr->pRoi[i-1], &dstRoi, rate);
                        Mat& dst = _dst.getMatRef(i);
                        dst.create(Size(dstRoi.width, dstRoi.height), type);
                        gPyr->pImage[i] = dst.data;
                        gPyr->pStep[i] = (int) dst.step;
                        gPyr->pRoi[i] = dstRoi;

                        if (ok >= 0) ok = pyrDownFunc(gPyr->pImage[i-1], gPyr->pStep[i-1], gPyr->pRoi[i-1],
                                                      gPyr->pImage[i], gPyr->pStep[i], gPyr->pRoi[i], gPyr->pState);

                        if (ok < 0)
                        {
                            pyrFreeFunc(gPyr->pState);
                            return false;
                        }
                        else
                        {
                            CV_IMPL_ADD(CV_IMPL_IPP);
                        }
                    }
                    pyrFreeFunc(gPyr->pState);
                }
                else
                {
                    ippiPyramidFree(gPyr);
                    return false;
                }
                ippiPyramidFree(gPyr);
            }
            return true;
        }
        return false;
    }
#else
    CV_UNUSED(_src); CV_UNUSED(_dst); CV_UNUSED(maxlevel); CV_UNUSED(borderType);
#endif
    return false;
}
}
#endif

void cv::buildPyramid( InputArray _src, OutputArrayOfArrays _dst, int maxlevel, int borderType )
{
    CV_INSTRUMENT_REGION();

    CV_Assert(borderType != BORDER_CONSTANT);

    if (_src.dims() <= 2 && _dst.isUMatVector())
    {
        UMat src = _src.getUMat();
        _dst.create( maxlevel + 1, 1, 0 );
        _dst.getUMatRef(0) = src;
        for( int i = 1; i <= maxlevel; i++ )
            pyrDown( _dst.getUMatRef(i-1), _dst.getUMatRef(i), Size(), borderType );
        return;
    }

    Mat src = _src.getMat();
    _dst.create( maxlevel + 1, 1, 0 );
    _dst.getMatRef(0) = src;

    int i=1;

    CV_IPP_RUN(((IPP_VERSION_X100 >= 810) && ((borderType & ~BORDER_ISOLATED) == BORDER_DEFAULT && (!_src.isSubmatrix() || ((borderType & BORDER_ISOLATED) != 0)))),
        ipp_buildpyramid( _src,  _dst,  maxlevel,  borderType));

    for( ; i <= maxlevel; i++ )
        pyrDown( _dst.getMatRef(i-1), _dst.getMatRef(i), Size(), borderType );
}

CV_IMPL void cvPyrDown( const void* srcarr, void* dstarr, int _filter )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( _filter == CV_GAUSSIAN_5x5 && src.type() == dst.type());
    cv::pyrDown( src, dst, dst.size() );
}

CV_IMPL void cvPyrUp( const void* srcarr, void* dstarr, int _filter )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);

    CV_Assert( _filter == CV_GAUSSIAN_5x5 && src.type() == dst.type());
    cv::pyrUp( src, dst, dst.size() );
}


CV_IMPL void
cvReleasePyramid( CvMat*** _pyramid, int extra_layers )
{
    if( !_pyramid )
        CV_Error( CV_StsNullPtr, "" );

    if( *_pyramid )
        for( int i = 0; i <= extra_layers; i++ )
            cvReleaseMat( &(*_pyramid)[i] );

    cvFree( _pyramid );
}


CV_IMPL CvMat**
cvCreatePyramid( const CvArr* srcarr, int extra_layers, double rate,
                 const CvSize* layer_sizes, CvArr* bufarr,
                 int calc, int filter )
{
    const float eps = 0.1f;
    uchar* ptr = 0;

    CvMat stub, *src = cvGetMat( srcarr, &stub );

    if( extra_layers < 0 )
        CV_Error( CV_StsOutOfRange, "The number of extra layers must be non negative" );

    int i, layer_step, elem_size = CV_ELEM_SIZE(src->type);
    cv::Size layer_size, size = cvGetMatSize(src);

    if( bufarr )
    {
        CvMat bstub, *buf;
        int bufsize = 0;

        buf = cvGetMat( bufarr, &bstub );
        bufsize = buf->rows*buf->cols*CV_ELEM_SIZE(buf->type);
        layer_size = size;
        for( i = 1; i <= extra_layers; i++ )
        {
            if( !layer_sizes )
            {
                layer_size.width = cvRound(layer_size.width*rate+eps);
                layer_size.height = cvRound(layer_size.height*rate+eps);
            }
            else
                layer_size = layer_sizes[i-1];
            layer_step = layer_size.width*elem_size;
            bufsize -= layer_step*layer_size.height;
        }

        if( bufsize < 0 )
            CV_Error( CV_StsOutOfRange, "The buffer is too small to fit the pyramid" );
        ptr = buf->data.ptr;
    }

    CvMat** pyramid = (CvMat**)cvAlloc( (extra_layers+1)*sizeof(pyramid[0]) );
    memset( pyramid, 0, (extra_layers+1)*sizeof(pyramid[0]) );

    pyramid[0] = cvCreateMatHeader( size.height, size.width, src->type );
    cvSetData( pyramid[0], src->data.ptr, src->step );
    layer_size = size;

    for( i = 1; i <= extra_layers; i++ )
    {
        if( !layer_sizes )
        {
            layer_size.width = cvRound(layer_size.width*rate + eps);
            layer_size.height = cvRound(layer_size.height*rate + eps);
        }
        else
            layer_size = layer_sizes[i];

        if( bufarr )
        {
            pyramid[i] = cvCreateMatHeader( layer_size.height, layer_size.width, src->type );
            layer_step = layer_size.width*elem_size;
            cvSetData( pyramid[i], ptr, layer_step );
            ptr += layer_step*layer_size.height;
        }
        else
            pyramid[i] = cvCreateMat( layer_size.height, layer_size.width, src->type );

        if( calc )
            cvPyrDown( pyramid[i-1], pyramid[i], filter );
            //cvResize( pyramid[i-1], pyramid[i], CV_INTER_LINEAR );
    }

    return pyramid;
}

/* End of file. */
