
/*
 * DEBUG: section 62    Generic Histogram
 * AUTHOR: Duane Wessels
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"
#include "StatHist.h"

#if HAVE_MATH_H
#include <math.h>
#endif
/* Local functions */
static StatHistBinDumper statHistBinDumper;

namespace Math
{
hbase_f Log;
hbase_f Exp;
hbase_f Null;
};

/* low level init, higher level functions has less params */
void
StatHist::init(unsigned int newCapacity, hbase_f * val_in_, hbase_f * val_out_, double newMin, double newMax)
{
    /* check before we divide to get scale_ */
    assert(val_in_(newMax - newMin) > 0);
    min_ = newMin;
    max_ = newMax;
    capacity_ = newCapacity;
    val_in = val_in_;
    val_out = val_out_;
    bins = static_cast<bins_type *>(xcalloc(capacity_, sizeof(bins_type)));
    scale_ = capacity_ / val_in(max_ - min_);
}

StatHist::StatHist(const StatHist &src) :
        capacity_(src.capacity_), min_(src.min_), max_(src.max_),
        scale_(src.scale_), val_in(src.val_in), val_out(src.val_out)
{
    if (src.bins!=NULL) {
        bins = static_cast<bins_type *>(xcalloc(src.capacity_, sizeof(bins_type)));
        memcpy(bins,src.bins,capacity_*sizeof(*bins));
    }
}

void
StatHist::count(double val)
{
    if (bins==NULL) //do not count before initialization or after destruction
        return;
    const unsigned int bin = findBin(val);
    ++bins[bin];
}

unsigned int
StatHist::findBin(double v)
{

    v -= min_;		/* offset */

    if (v <= 0.0)		/* too small */
        return 0;

    unsigned int bin;
    double tmp_bin=floor(scale_ * val_in(v) + 0.5);

    if (tmp_bin < 0.0) // should not happen
        return 0;
    bin = static_cast <unsigned int>(tmp_bin);

    if (bin >= capacity_)	/* too big */
        bin = capacity_ - 1;

    return bin;
}

double
StatHist::val(unsigned int bin) const
{
    return val_out((double) bin / scale_) + min_;
}

double
statHistDeltaMedian(const StatHist & A, const StatHist & B)
{
    return statHistDeltaPctile(A, B, 0.5);
}

double
statHistDeltaPctile(const StatHist & A, const StatHist & B, double pctile)
{
    return A.deltaPctile(B, pctile);
}

double
StatHist::deltaPctile(const StatHist & B, double pctile) const
{
    unsigned int i;
    bins_type s1 = 0;
    bins_type h = 0;
    bins_type a = 0;
    bins_type b = 0;
    unsigned int I = 0;
    unsigned int J = capacity_;
    unsigned int K;
    double f;

    assert(capacity_ == B.capacity_);

    int *D = static_cast<int *>(xcalloc(capacity_, sizeof(int)));

    for (i = 0; i < capacity_; ++i) {
        D[i] = B.bins[i] - bins[i];
        assert(D[i] >= 0);
    }

    for (i = 0; i < capacity_; ++i)
        s1 += D[i];

    h = int(s1 * pctile);

    for (i = 0; i < capacity_; ++i) {
        J = i;
        b += D[J];

        if (a <= h && h <= b)
            break;

        I = i;

        a += D[I];
    }

    xfree(D);

    if (s1 == 0)
        return 0.0;

    if (a > h)
        return 0.0;

    if (a >= b)
        return 0.0;

    if (I >= J)
        return 0.0;

    f = (h - a) / (b - a);

    K = (unsigned int) floor(f * (double) (J - I) + I);

    return val(K);
}

static void
statHistBinDumper(StoreEntry * sentry, int idx, double val, double size, int count)
{
    if (count)
        storeAppendPrintf(sentry, "\t%3d/%f\t%d\t%f\n",
                          idx, val, count, count / size);
}

void
StatHist::dump(StoreEntry * sentry, StatHistBinDumper * bd) const
{
    double left_border = min_;

    if (!bd)
        bd = statHistBinDumper;

    for (unsigned int i = 0; i < capacity_; ++i) {
        const double right_border = val(i + 1);
        assert(right_border - left_border > 0.0);
        bd(sentry, i, left_border, right_border - left_border, bins[i]);
        left_border = right_border;
    }
}

/* log based histogram */
double
Math::Log(double x)
{
    assert((x + 1.0) >= 0.0);
    return log(x + 1.0);
}

double
Math::Exp(double x)
{
    return exp(x) - 1.0;
}

void
StatHist::logInit(unsigned int capacity, double min, double max)
{
    init(capacity, Math::Log, Math::Exp, min, max);
}

/* linear histogram for enums */
/* we want to be have [-1,last_enum+1] range to track out of range enums */
double
Math::Null(double x)
{
    return x;
}

void
StatHist::enumInit(unsigned int last_enum)
{
    init(last_enum + 3, Math::Null, Math::Null, -1.0, (2.0 + last_enum));
}

void
statHistEnumDumper(StoreEntry * sentry, int idx, double val, double size, int count)
{
    if (count)
        storeAppendPrintf(sentry, "%2d\t %5d\t %5d\n",
                          idx, (int) val, count);
}

void
statHistIntDumper(StoreEntry * sentry, int idx, double val, double size, int count)
{
    if (count)
        storeAppendPrintf(sentry, "%9d\t%9d\n", (int) val, count);
}
