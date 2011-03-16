// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+
/// Built-in function definitions

#include "bfunc.h"

#include <boost/math/special_functions/gamma.hpp>

#include "voigt.h"
#include "numfuncs.h"

using namespace std;
using boost::math::lgamma;


#define CALCULATE_VALUE_BEGIN(NAME) \
void NAME::calculate_value_in_range(vector<realt> const &xx, vector<realt> &yy,\
                                    int first, int last) const\
{\
    for (int i = first; i < last; ++i) {\
        realt x = xx[i];


#define CALCULATE_VALUE_END(VAL) \
        yy[i] += (VAL);\
    }\
}

#define CALCULATE_DERIV_BEGIN(NAME) \
void NAME::calculate_value_deriv_in_range(vector<realt> const &xx, \
                                          vector<realt> &yy, \
                                          vector<realt> &dy_da, \
                                          bool in_dx, \
                                          int first, int last) const \
{ \
    int dyn = dy_da.size() / xx.size(); \
    vector<realt> dy_dv(nv()); \
    for (int i = first; i < last; ++i) { \
        realt x = xx[i]; \
        realt dy_dx;


#define CALCULATE_DERIV_END(VAL) \
        if (!in_dx) { \
            yy[i] += (VAL); \
            v_foreach (Multi, j, multi_) \
                dy_da[dyn*i+j->p] += dy_dv[j->n] * j->mult;\
            dy_da[dyn*i+dyn-1] += dy_dx;\
        }\
        else {  \
            v_foreach (Multi, j, multi_) \
                dy_da[dyn*i+j->p] += dy_da[dyn*i+dyn-1] * dy_dv[j->n]*j->mult;\
        } \
    } \
}


///////////////////////////////////////////////////////////////////////

void FuncConstant::calculate_value_in_range(vector<realt> const&/*xx*/,
                                            vector<realt>& yy,
                                            int first, int last) const
{
    for (int i = first; i < last; ++i)
        yy[i] += av_[0];
}

CALCULATE_DERIV_BEGIN(FuncConstant)
    (void) x;
    dy_dv[0] = 1.;
    dy_dx = 0;
CALCULATE_DERIV_END(av_[0])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncLinear)
CALCULATE_VALUE_END(av_[0] + x*av_[1])

CALCULATE_DERIV_BEGIN(FuncLinear)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dx = av_[1];
CALCULATE_DERIV_END(av_[0] + x*av_[1])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncQuadratic)
CALCULATE_VALUE_END(av_[0] + x*av_[1] + x*x*av_[2])

CALCULATE_DERIV_BEGIN(FuncQuadratic)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dv[2] = x*x;
    dy_dx = av_[1] + 2*x*av_[2];
CALCULATE_DERIV_END(av_[0] + x*av_[1] + x*x*av_[2])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncCubic)
CALCULATE_VALUE_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3])

CALCULATE_DERIV_BEGIN(FuncCubic)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dv[2] = x*x;
    dy_dv[3] = x*x*x;
    dy_dx = av_[1] + 2*x*av_[2] + 3*x*x*av_[3];
CALCULATE_DERIV_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncPolynomial4)
CALCULATE_VALUE_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3]
                                          + x*x*x*x*av_[4])

CALCULATE_DERIV_BEGIN(FuncPolynomial4)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dv[2] = x*x;
    dy_dv[3] = x*x*x;
    dy_dv[4] = x*x*x*x;
    dy_dx = av_[1] + 2*x*av_[2] + 3*x*x*av_[3] + 4*x*x*x*av_[4];
CALCULATE_DERIV_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3]
                                      + x*x*x*x*av_[4])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncPolynomial5)
CALCULATE_VALUE_END(av_[0] + x*av_[1] + x*x*av_[2]
                       + x*x*x*av_[3] + x*x*x*x*av_[4] + x*x*x*x*x*av_[5])

CALCULATE_DERIV_BEGIN(FuncPolynomial5)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dv[2] = x*x;
    dy_dv[3] = x*x*x;
    dy_dv[4] = x*x*x*x;
    dy_dv[5] = x*x*x*x*x;
    dy_dx = av_[1] + 2*x*av_[2] + 3*x*x*av_[3] + 4*x*x*x*av_[4]
               + 5*x*x*x*x*av_[5];
CALCULATE_DERIV_END(av_[0] + x*av_[1] + x*x*av_[2]
                          + x*x*x*av_[3] + x*x*x*x*av_[4] + x*x*x*x*x*av_[5])

///////////////////////////////////////////////////////////////////////

CALCULATE_VALUE_BEGIN(FuncPolynomial6)
CALCULATE_VALUE_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3] +
                        x*x*x*x*av_[4] + x*x*x*x*x*av_[5] + x*x*x*x*x*x*av_[6])

CALCULATE_DERIV_BEGIN(FuncPolynomial6)
    dy_dv[0] = 1.;
    dy_dv[1] = x;
    dy_dv[2] = x*x;
    dy_dv[3] = x*x*x;
    dy_dv[4] = x*x*x*x;
    dy_dv[5] = x*x*x*x*x;
    dy_dv[6] = x*x*x*x*x*x;
    dy_dx = av_[1] + 2*x*av_[2] + 3*x*x*av_[3] + 4*x*x*x*av_[4]
                + 5*x*x*x*x*av_[5] + 6*x*x*x*x*x*av_[6];
CALCULATE_DERIV_END(av_[0] + x*av_[1] + x*x*av_[2] + x*x*x*av_[3] +
                        x*x*x*x*av_[4] + x*x*x*x*x*av_[5] + x*x*x*x*x*x*av_[6])

///////////////////////////////////////////////////////////////////////

void FuncGaussian::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncGaussian)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
CALCULATE_VALUE_END(av_[0] * ex)

CALCULATE_DERIV_BEGIN(FuncGaussian)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
    dy_dv[0] = ex;
    realt dcenter = 2 * M_LN2 * av_[0] * ex * xa1a2 / av_[2];
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2;
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0]*ex)

bool FuncGaussian::get_nonzero_range(double level,
                                     realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        realt w = sqrt(log(fabs(av_[0]/level)) / M_LN2) * av_[2];
        left = av_[1] - w;
        right = av_[1] + w;
    }
    return true;
}

bool FuncGaussian::get_area(realt* a) const
{
    *a = av_[0] * fabs(av_[2]) * sqrt(M_PI / M_LN2);
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncSplitGaussian::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
    if (fabs(av_[3]) < epsilon)
        av_[3] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncSplitGaussian)
    realt hwhm = (x < av_[1] ? av_[2] : av_[3]);
    realt xa1a2 = (x - av_[1]) / hwhm;
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
CALCULATE_VALUE_END(av_[0] * ex)

CALCULATE_DERIV_BEGIN(FuncSplitGaussian)
    realt hwhm = (x < av_[1] ? av_[2] : av_[3]);
    realt xa1a2 = (x - av_[1]) / hwhm;
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
    dy_dv[0] = ex;
    realt dcenter = 2 * M_LN2 * av_[0] * ex * xa1a2 / hwhm;
    dy_dv[1] = dcenter;
    if (x < av_[1]) {
        dy_dv[2] = dcenter * xa1a2;
        dy_dv[3] = 0;
    }
    else {
        dy_dv[2] = 0;
        dy_dv[3] = dcenter * xa1a2;
    }
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0]*ex)

bool FuncSplitGaussian::get_nonzero_range(double level,
                                          realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        realt w1 = sqrt (log (fabs(av_[0]/level)) / M_LN2) * av_[2];
        realt w2 = sqrt (log (fabs(av_[0]/level)) / M_LN2) * av_[3];
        left = av_[1] - w1;
        right = av_[1] + w2;
    }
    return true;
}

bool FuncSplitGaussian::get_area(realt* a) const
{
    *a = av_[0] * (fabs(av_[2])+fabs(av_[3])) / 2. * sqrt(M_PI/M_LN2);
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncLorentzian::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncLorentzian)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt inv_denomin = 1. / (1 + xa1a2 * xa1a2);
CALCULATE_VALUE_END(av_[0] * inv_denomin)

CALCULATE_DERIV_BEGIN(FuncLorentzian)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt inv_denomin = 1. / (1 + xa1a2 * xa1a2);
    dy_dv[0] = inv_denomin;
    realt dcenter = 2 * av_[0] * xa1a2 / av_[2] * inv_denomin * inv_denomin;
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2;
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0] * inv_denomin)

bool FuncLorentzian::get_nonzero_range(double level,
                                       realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        realt w = sqrt(fabs(av_[0]/level) - 1) * av_[2];
        left = av_[1] - w;
        right = av_[1] + w;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncPearson7::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
    if (av_.size() != 5)
        av_.resize(5);
    // not checking for av_[3]>0.5 nor even >0
    av_[4] = pow(2, 1. / av_[3]) - 1;
}

CALCULATE_VALUE_BEGIN(FuncPearson7)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt xa1a2sq = xa1a2 * xa1a2;
    realt pow_2_1_a3_1 = av_[4]; //pow (2, 1. / a3) - 1;
    realt denom_base = 1 + xa1a2sq * pow_2_1_a3_1;
    realt inv_denomin = pow(denom_base, - av_[3]);
CALCULATE_VALUE_END(av_[0] * inv_denomin)

CALCULATE_DERIV_BEGIN(FuncPearson7)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt xa1a2sq = xa1a2 * xa1a2;
    realt pow_2_1_a3_1 = av_[4]; //pow (2, 1. / a3) - 1;
    realt denom_base = 1 + xa1a2sq * pow_2_1_a3_1;
    realt inv_denomin = pow(denom_base, - av_[3]);
    dy_dv[0] = inv_denomin;
    realt dcenter = 2 * av_[0] * av_[3] * pow_2_1_a3_1 * xa1a2 * inv_denomin /
                                                      (denom_base * av_[2]);
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2;
    dy_dv[3] = av_[0] * inv_denomin * (M_LN2 * (pow_2_1_a3_1 + 1)
                       * xa1a2sq / (denom_base * av_[3]) - log(denom_base));
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0] * inv_denomin)


bool FuncPearson7::get_nonzero_range(double level,
                                     realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        realt t = (pow(fabs(av_[0]/level), 1./av_[3]) - 1)
                  / (pow(2, 1./av_[3]) - 1);
        realt w = sqrt(t) * av_[2];
        left = av_[1] - w;
        right = av_[1] + w;
    }
    return true;
}

bool FuncPearson7::get_area(realt* a) const
{
    if (av_[3] <= 0.5)
        return false;
    realt g = exp(lgamma(av_[3] - 0.5) - lgamma(av_[3]));
    //in f_val_precomputations(): av_[4] = pow (2, 1. / a3) - 1;
    *a = av_[0] * 2 * fabs(av_[2]) * sqrt(M_PI) * g / (2 * sqrt(av_[4]));
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncSplitPearson7::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
    if (fabs(av_[3]) < epsilon)
        av_[3] = epsilon;
    if (av_.size() != 8)
        av_.resize(8);
    // not checking for av_[3]>0.5 nor even >0
    av_[6] = pow(2, 1. / av_[4]) - 1;
    av_[7] = pow(2, 1. / av_[5]) - 1;
}

CALCULATE_VALUE_BEGIN(FuncSplitPearson7)
    int lr = x < av_[1] ? 0 : 1;
    realt xa1a2 = (x - av_[1]) / av_[2+lr];
    realt xa1a2sq = xa1a2 * xa1a2;
    realt pow_2_1_a3_1 = av_[6+lr]; //pow(2, 1./shape) - 1;
    realt denom_base = 1 + xa1a2sq * pow_2_1_a3_1;
    realt inv_denomin = pow(denom_base, - av_[4+lr]);
CALCULATE_VALUE_END(av_[0] * inv_denomin)

CALCULATE_DERIV_BEGIN(FuncSplitPearson7)
    int lr = x < av_[1] ? 0 : 1;
    realt hwhm = av_[2+lr];
    realt shape = av_[4+lr];
    realt xa1a2 = (x - av_[1]) / hwhm;
    realt xa1a2sq = xa1a2 * xa1a2;
    realt pow_2_1_a3_1 = av_[6+lr]; //pow(2, 1./shape) - 1;
    realt denom_base = 1 + xa1a2sq * pow_2_1_a3_1;
    realt inv_denomin = pow (denom_base, -shape);
    dy_dv[0] = inv_denomin;
    realt dcenter = 2 * av_[0] * shape * pow_2_1_a3_1 * xa1a2 * inv_denomin /
                                                      (denom_base * hwhm);
    dy_dv[1] = dcenter;
    dy_dv[2] = dy_dv[3] = dy_dv[4] = dy_dv[5] = 0;
    dy_dv[2+lr] = dcenter * xa1a2;
    dy_dv[4+lr] = av_[0] * inv_denomin * (M_LN2 * (pow_2_1_a3_1 + 1)
                           * xa1a2sq / (denom_base * shape) - log(denom_base));
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0] * inv_denomin)


bool FuncSplitPearson7::get_nonzero_range(double level,
                                          realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        realt t1 = (pow(fabs(av_[0]/level), 1./av_[4]) - 1)
                                                / (pow(2, 1./av_[4]) - 1);
        realt w1 = sqrt(t1) * av_[2];
        realt t2 = (pow(fabs(av_[0]/level), 1./av_[5]) - 1)
                                                / (pow(2, 1./av_[5]) - 1);
        realt w2 = sqrt(t2) * av_[3];
        left = av_[1] - w1;
        right = av_[1] + w2;
    }
    return true;
}

bool FuncSplitPearson7::get_area(realt* a) const
{
    if (av_[4] <= 0.5 || av_[5] <= 0.5)
        return false;
    realt g1 = exp(lgamma(av_[4] - 0.5) - lgamma(av_[4]));
    realt g2 = exp(lgamma(av_[5] - 0.5) - lgamma(av_[5]));
    *a =   av_[0] * fabs(av_[2]) * sqrt(M_PI) * g1 / (2 * sqrt(av_[6]))
         + av_[0] * fabs(av_[3]) * sqrt(M_PI) * g2 / (2 * sqrt(av_[7]));
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncPseudoVoigt::more_precomputations()
{
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncPseudoVoigt)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
    realt lor = 1. / (1 + xa1a2 * xa1a2);
    realt without_height =  (1-av_[3]) * ex + av_[3] * lor;
CALCULATE_VALUE_END(av_[0] * without_height)

CALCULATE_DERIV_BEGIN(FuncPseudoVoigt)
    realt xa1a2 = (x - av_[1]) / av_[2];
    realt ex = exp(- M_LN2 * xa1a2 * xa1a2);
    realt lor = 1. / (1 + xa1a2 * xa1a2);
    realt without_height =  (1-av_[3]) * ex + av_[3] * lor;
    dy_dv[0] = without_height;
    realt dcenter = 2 * av_[0] * xa1a2 / av_[2]
                        * (av_[3]*lor*lor + (1-av_[3])*M_LN2*ex);
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2;
    dy_dv[3] =  av_[0] * (lor - ex);
    dy_dx = -dcenter;
CALCULATE_DERIV_END(av_[0] * without_height)

bool FuncPseudoVoigt::get_nonzero_range(double level,
                                        realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        // neglecting Gaussian part and adding 4.0 to compensate it
        realt w = (sqrt (av_[3] * fabs(av_[0]/level) - 1) + 4.) * av_[2];
        left = av_[1] - w;
        right = av_[1] + w;
    }
    return true;
}

bool FuncPseudoVoigt::get_area(realt* a) const
{
    *a = av_[0] * fabs(av_[2])
              * ((av_[3] * M_PI) + (1 - av_[3]) * sqrt(M_PI / M_LN2));
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncVoigt::more_precomputations()
{
    if (av_.size() != 6)
        av_.resize(6);
    float k, l, dkdx, dkdy;
    humdev(0, fabs(av_[3]), k, l, dkdx, dkdy);
    av_[4] = 1. / k;
    av_[5] = dkdy / k;

    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncVoigt)
    // humdev/humlik routines require with y (a3 here) parameter >0.
    float k;
    realt xa1a2 = (x - av_[1]) / av_[2];
    k = humlik(xa1a2, fabs(av_[3]));
CALCULATE_VALUE_END(av_[0] * av_[4] * k)

CALCULATE_DERIV_BEGIN(FuncVoigt)
    // humdev/humlik routines require with y (a3 here) parameter >0.
    // here fabs(av_[3]) is used, and dy_dv[3] is negated if av_[3]<0.
    float k;
    realt xa1a2 = (x-av_[1]) / av_[2];
    realt a0a4 = av_[0] * av_[4];
    float l, dkdx, dkdy;
    humdev(xa1a2, fabs(av_[3]), k, l, dkdx, dkdy);
    dy_dv[0] = av_[4] * k;
    realt dcenter = -a0a4 * dkdx / av_[2];
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2;
    dy_dv[3] = a0a4 * (dkdy - k * av_[5]);
    if (av_[3] < 0)
        dy_dv[3] = -dy_dv[3];
    dy_dx = -dcenter;
CALCULATE_DERIV_END(a0a4 * k)

bool FuncVoigt::get_nonzero_range(double level,
                                  realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        //TODO estimate Voigt's non-zero range
        return false;
    }
    return true;
}

///estimation according to
/// http://en.wikipedia.org/w/index.php?title=Voigt_profile&oldid=115518205
///
/// a2 = sqrt(2) * sigma
/// a3 = gamma / (sqrt(2) * sigma)
///
/// sigma = a2 / sqrt(2)
/// gamma = a2 * a3
static realt voigt_fwhm(realt a2, realt a3)
{
    realt sigma = fabs(a2) / M_SQRT2;
    realt gamma = fabs(a2) * a3;

    realt fG = 2 * sigma * sqrt(2 * M_LN2);
    realt fL = 2 * gamma;

    realt fV = 0.5346 * fL + sqrt(0.2166 * fL * fL + fG * fG);
    return fV;
}

bool FuncVoigt::get_fwhm(realt* a) const
{
    *a = voigt_fwhm(av_[2], av_[3]);
    return true;
}

bool FuncVoigt::get_area(realt* a) const
{
    *a = av_[0] * fabs(av_[2] * sqrt(M_PI) * av_[4]);
    return true;
}

const vector<string>& FuncVoigt::get_other_prop_names() const
{
    static const vector<string> p = vector2(string("GaussianFWHM"),
                                            string("LorentzianFWHM"));
    return p;
}

realt FuncVoigt::get_other_prop(string const& name) const
{
    if (name == "GaussianFWHM") {
        realt sigma = fabs(av_[2]) / M_SQRT2;
        return 2 * sigma * sqrt(2 * M_LN2);
    }
    else if (name == "LorentzianFWHM") {
        realt gamma = fabs(av_[2]) * av_[3];
        return 2 * gamma;
    }
    else
        return 0.;
}

///////////////////////////////////////////////////////////////////////

void FuncVoigtA::more_precomputations()
{
    if (av_.size() != 6)
        av_.resize(6);
    av_[4] = 1. / humlik(0, fabs(av_[3]));

    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
}

CALCULATE_VALUE_BEGIN(FuncVoigtA)
    // humdev/humlik routines require with y (a3 here) parameter >0.
    float k;
    realt xa1a2 = (x - av_[1]) / av_[2];
    k = humlik(xa1a2, fabs(av_[3]));
CALCULATE_VALUE_END(av_[0] / (sqrt(M_PI) * av_[2]) * k)

CALCULATE_DERIV_BEGIN(FuncVoigtA)
    // humdev/humlik routines require with y (a3 here) parameter >0.
    // here fabs(av_[3]) is used, and dy_dv[3] is negated if av_[3]<0.
    float k;
    realt xa1a2 = (x-av_[1]) / av_[2];
    realt f = av_[0] / (sqrt(M_PI) * av_[2]);
    float l, dkdx, dkdy;
    humdev(xa1a2, fabs(av_[3]), k, l, dkdx, dkdy);
    dy_dv[0] = k / (sqrt(M_PI) * av_[2]);
    realt dcenter = -f * dkdx / av_[2];
    dy_dv[1] = dcenter;
    dy_dv[2] = dcenter * xa1a2 - f * k / av_[2];
    dy_dv[3] = f * dkdy;
    if (av_[3] < 0)
        dy_dv[3] = -dy_dv[3];
    dy_dx = -dcenter;
CALCULATE_DERIV_END(f * k)

bool FuncVoigtA::get_nonzero_range(double level,
                                   realt &left, realt &right) const
{
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        //TODO estimate Voigt's non-zero range
        return false;
    }
    return true;
}

bool FuncVoigtA::get_fwhm(realt* a) const
{
    *a = voigt_fwhm(av_[2], av_[3]);
    return true;
}

bool FuncVoigtA::get_height(realt* a) const
{
    *a = av_[0] / fabs(av_[2] * sqrt(M_PI) * av_[4]);
    return true;
}


///////////////////////////////////////////////////////////////////////

void FuncEMG::more_precomputations()
{
}

bool FuncEMG::get_nonzero_range(double/*level*/,
                                realt&/*left*/, realt&/*right*/) const
{
    return false;
}

CALCULATE_VALUE_BEGIN(FuncEMG)
    realt a = av_[0];
    realt bx = av_[1] - x;
    realt c = av_[2];
    realt d = av_[3];
    realt fact = a*c*sqrt(2*M_PI)/(2*d);
    realt ex = exp(bx/d + c*c/(2*d*d));
    //realt erf_arg = bx/(M_SQRT2*c) + c/(M_SQRT2*d);
    realt erf_arg = (bx/c + c/d) / M_SQRT2;
    realt t = fact * ex * (d >= 0 ? erfc(erf_arg) : -erfc(-erf_arg));
    //realt t = fact * ex * (d >= 0 ? 1-erf(erf_arg) : -1-erf(erf_arg));
CALCULATE_VALUE_END(t)

CALCULATE_DERIV_BEGIN(FuncEMG)
    realt a = av_[0];
    realt bx = av_[1] - x;
    realt c = av_[2];
    realt d = av_[3];
    realt cs2d = c/(M_SQRT2*d);
    realt cc = c*sqrt(M_PI/2)/d;
    realt ex = exp(bx/d + cs2d*cs2d); //==exp((c^2+2bd-2dx) / 2d^2)
    realt bx2c = bx/(M_SQRT2*c);
    realt erf_arg = bx2c + cs2d; //== (c*c+b*d-d*x)/(M_SQRT2*c*d);
    //realt er = erf(erf_arg);
    //realt d_sign = d >= 0 ? 1 : -1;
    //realt ser = d_sign - er;
    realt ser = (d >= 0 ? erfc(erf_arg) : -erfc(-erf_arg));
    realt t = cc * ex * ser;
    realt eee = exp(erf_arg*erf_arg);
    dy_dv[0] = t;
    dy_dv[1] = -a/d * ex / eee + a*t/d;
    dy_dv[2] = - a/(2*c*d*d*d)*exp(-bx2c*bx2c)
                  * (2*d*(c*c-bx*d) - sqrt(2*M_PI)*c*(c*c+d*d) * eee * ser);
    //dy_dv[3] = a*c/(d*d*d)*ex * (c/eee
    //                             - d_sign * (c*cc + sqrt(M_PI/2)*(b+d-x))
    //                             + sqrt(M_PI/2) * (c*c/d + b+d-x) * er);
    dy_dv[3] = a*c/(d*d*d)*ex * (c/eee - ser * (c*cc + sqrt(M_PI/2)*(bx+d)));
    dy_dx = - dy_dv[1];
CALCULATE_DERIV_END(a*t)

///////////////////////////////////////////////////////////////////////

bool FuncDoniachSunjic::get_nonzero_range(double/*level*/,
                                          realt&/*left*/, realt&/*right*/) const
{
    return false;
}

CALCULATE_VALUE_BEGIN(FuncDoniachSunjic)
    realt h = av_[0];
    realt a = av_[1];
    realt F = av_[2];
    realt xE = x - av_[3];
    realt t = h * cos(M_PI*a/2 + (1-a)*atan(xE/F)) / pow(F*F+xE*xE, (1-a)/2);
CALCULATE_VALUE_END(t)

CALCULATE_DERIV_BEGIN(FuncDoniachSunjic)
    realt h = av_[0];
    realt a = av_[1];
    realt F = av_[2];
    realt xE = x - av_[3];
    realt fe2 = F*F+xE*xE;
    realt ac = 1-a;
    realt p = pow(fe2, -ac/2);
    realt at = atan(xE/F);
    realt cos_arg = M_PI*a/2 + ac*at;
    realt co = cos(cos_arg);
    realt si = sin(cos_arg);
    realt t = co * p;
    dy_dv[0] = t;
    dy_dv[1] = h * p * (co/2 * log(fe2) + (at-M_PI/2) * si);
    dy_dv[2] = h * ac*p/fe2 * (xE*si - F*co);
    dy_dv[3] = h * ac*p/fe2 * (xE*co + si*F);
    dy_dx = -dy_dv[3];
CALCULATE_DERIV_END(h*t)
///////////////////////////////////////////////////////////////////////


CALCULATE_VALUE_BEGIN(FuncPielaszekCube)
    realt height = av_[0];
    realt center = av_[1];
    realt R = av_[2];
    realt s = av_[3];
    realt s2 = s*s;
    realt s4 = s2*s2;
    realt R2 = R*R;

    realt q = (x-center);
    realt q2 = q*q;
    realt t = height * (-3*R*(-1 - (R2*(-1 +
                          pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))
                          * cos(2*(-1.5 + R2/(2.*s2)) * atan((q*s2)/R))))/
           (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
      (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))* s2);
CALCULATE_VALUE_END(t)

CALCULATE_DERIV_BEGIN(FuncPielaszekCube)
    realt height = av_[0];
    realt center = av_[1];
    realt R = av_[2];
    realt s = av_[3];
    realt s2 = s*s;
    realt s3 = s*s2;
    realt s4 = s2*s2;
    realt R2 = R*R;
    realt R4 = R2*R2;
    realt R3 = R*R2;

    realt q = (x-center);
    realt q2 = q*q;
    realt t = (-3*R*(-1 - (R2*(-1 +
                              pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))
                              * cos(2*(-1.5 + R2/(2.*s2)) * atan((q*s2)/R))))/
               (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
          (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))* s2);

    realt dcenter = height * (
            (3*sqrt(2/M_PI)*R*(-1 -
                        (R2* (-1 + pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
   (q*q2*(-0.5 + R2/(2.*s2))*s2) - (3*R*((R2*(-1 +
            pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (q*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4) -
       (R2*((2*q*(1.5 - R2/(2.*s2))* s4*
               pow(1 + (q2*s4)/R2, 0.5 - R2/(2.*s2))*
               cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R)))/R2 -
            (2*(-1.5 + R2/(2.*s2))*s2* pow(1 + (q2*s4)/R2,
                0.5 - R2/(2.*s2))* sin(2*(-1.5 + R2/(2.*s2))*
                 atan((q*s2)/R)))/R))/
        (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
   (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))* s2));

    realt dR = height * (
        (3*R2*(-1 - (R2* (-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))))/ (2.*q2*(-1.5 + R2/(2.*s2))*
          (-1 + R2/(2.*s2))*s4)))/ (sqrt(2*M_PI)*q2*pow(-0.5 + R2/(2.*s2),2)*
     s4) - (3*(-1 - (R2*(-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))))/ (2.*q2*(-1.5 + R2/(2.*s2))*
          (-1 + R2/(2.*s2))*s4)))/ (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))*
     s2) - (3*R*((R3* (-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))))/ (2.*q2*(-1.5 + R2/(2.*s2))*
          pow(-1 + R2/(2.*s2),2)*s4*s2) + (R3*(-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*pow(-1.5 + R2/(2.*s2),2)* (-1 + R2/(2.*s2))*(s4*s2)) -
       (R*(-1 + pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4) -
       (R2*(pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))*
             ((-2*q2*(1.5 - R2/(2.*s2))* s4)/ (R3*
                  (1 + (q2*s4)/R2)) - (R*log(1 + (q2*s4)/R2))/ s2) +
            pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))* ((2*q*(-1.5 + R2/(2.*s2))*
                  s2)/ (R2* (1 + (q2*s4)/R2)) - (2*R*atan((q*s2)/R))/s2)*
             sin(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
   (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))* s2));

    realt ds = height * (
            (-3*R3*(-1 - (R2* (-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))))/ (2.*q2*(-1.5 + R2/(2.*s2))*
          (-1 + R2/(2.*s2))*s4)))/
   (sqrt(2*M_PI)*q2*pow(-0.5 + R2/(2.*s2),2)* (s4*s)) + (3*sqrt(2/M_PI)*R*
     (-1 - (R2*(-1 + pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
   (q2*(-0.5 + R2/(2.*s2))*s3) - (3*R*(-(R4*(-1 +
             pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
              cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*(-1.5 + R2/(2.*s2))* pow(-1 + R2/(2.*s2),2)*(s4*s3)) -
       (R4*(-1 + pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             cos(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*pow(-1.5 + R2/(2.*s2),2)* (-1 + R2/(2.*s2))*(s4*s3)) +
       (2*R2*(-1 + pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))))/ (q2*(-1.5 + R2/(2.*s2))*
          (-1 + R2/(2.*s2))*(s4*s)) - (R2*(pow(1 + (q2*s4)/R2,
              1.5 - R2/(2.*s2))* cos(2*(-1.5 + R2/(2.*s2))*
               atan((q*s2)/R))* ((4*q2*(1.5 - R2/(2.*s2))* s3)/
                (R2* (1 + (q2*s4)/R2)) + (R2*log(1 +
                    (q2*s4)/R2))/ s3) +
            pow(1 + (q2*s4)/R2, 1.5 - R2/(2.*s2))*
             ((-4*q*(-1.5 + R2/(2.*s2))*s)/ (R*(1 + (q2*s4)/R2)) +
               (2*R2*atan((q*s2)/R))/ s3)*
             sin(2*(-1.5 + R2/(2.*s2))* atan((q*s2)/R))))/
        (2.*q2*(-1.5 + R2/(2.*s2))* (-1 + R2/(2.*s2))*s4)))/
   (sqrt(2*M_PI)*q2*(-0.5 + R2/(2.*s2))* s2));

    dy_dv[0] = t;
    dy_dv[1] = -dcenter;
    dy_dv[2] = dR;
    dy_dv[3] = ds;
    dy_dx = dcenter;
CALCULATE_DERIV_END(height*t);

///////////////////////////////////////////////////////////////////////

// Implemented by Mirko Scholz. The formula is taken from:
// Bingemann, D.; Ernsting, N. P. J. Chem. Phys. 1995, 102, 2691–2700.

void FuncLogNormal::more_precomputations()
{
    if (av_.size() != 4)
        av_.resize(4);
    if (fabs(av_[2]) < epsilon)
        av_[2] = epsilon;
    if (fabs(av_[3]) < epsilon)
        av_[3] = 0.001;
}

CALCULATE_VALUE_BEGIN(FuncLogNormal)
    realt a = 2.0 * av_[3] * (x - av_[1]) / av_[2];
    realt ex = 0.0;
    if (a > -1.0) {
        realt b = log(1 + a) / av_[3];
        ex = av_[0] * exp(-M_LN2 * b * b);
    }
CALCULATE_VALUE_END(ex)

CALCULATE_DERIV_BEGIN(FuncLogNormal)
    realt a = 2.0 * av_[3] * (x - av_[1]) / av_[2];
    realt ex;
    if (a > -1.0) {
        realt b = log(1 + a) / av_[3];
        ex = exp(-M_LN2 * b * b);
        dy_dv[0] = ex;
        ex *= av_[0];
        dy_dv[1] = 4.0*M_LN2/(av_[2]*(a+1))*ex*b;
        dy_dv[2] = 4.0*(x-av_[1])*M_LN2/(av_[2]*av_[2]*(a+1))*ex*b;
        dy_dv[3] = ex*(2.0*M_LN2*b*b/av_[3]
            -4.0*(x-av_[1])*log(a+1)*M_LN2/(av_[2]*av_[3]*av_[3]*(a+1)));
        dy_dx = -4.0*M_LN2/(av_[2]*(a+1))*ex*b;
    }
    else {
        ex = 0.0;
        dy_dv[0] = 0.0;
        dy_dv[1] = 0.0;
        dy_dv[2] = 0.0;
        dy_dv[3] = 0.0;
        dy_dx = 0.0;
    }
CALCULATE_DERIV_END(ex)

bool FuncLogNormal::get_nonzero_range(double level,
                                      realt &left, realt &right) const
{ /* untested */
    if (level == 0)
        return false;
    else if (fabs(level) >= fabs(av_[0]))
        left = right = 0;
    else {
        //realt w = sqrt (log (fabs(av_[0]/level)) / M_LN2) * av_[2];
        realt w1 = (1-exp(sqrt(log(fabs(av_[0]/level))/M_LN2)*av_[3]))*av_[2]
            /2.0/av_[3]+av_[1];
        realt w0 = (1-exp(-sqrt(log(fabs(av_[0]/level))/M_LN2)*av_[3]))*av_[2]
            /2.0/av_[3]+av_[1];
        if (w1>w0) {
            left = w0;
            right = w1;
        }
        else {
            left = w1;
            right = w0;
        }
    }
    return true;
}

//cf. eq. 28 of Maroncelli, M.; Fleming, G.R. J. Phys. Chem. 1987, 86, 6221-6239
bool FuncLogNormal::get_fwhm(realt* a) const
{
   *a = av_[2]*sinh(av_[3])/av_[3];
   return true;
}

bool FuncLogNormal::get_area(realt* a) const
{
    *a = av_[0]/sqrt(M_LN2/M_PI) / (2.0/av_[2]) / exp(-av_[3]*av_[3]/4.0/M_LN2);
    return true;
}

///////////////////////////////////////////////////////////////////////

void FuncSpline::more_precomputations()
{
    q_.resize(nv() / 2);
    for (size_t i = 0; i < q_.size(); ++i) {
        q_[i].x = av_[2*i];
        q_[i].y = av_[2*i+1];
    }
    prepare_spline_interpolation(q_);

}

CALCULATE_VALUE_BEGIN(FuncSpline)
    realt t = get_spline_interpolation(q_, x);
CALCULATE_VALUE_END(t)

CALCULATE_DERIV_BEGIN(FuncSpline)
    dy_dx = 0; // unused
    realt t = get_spline_interpolation(q_, x);
CALCULATE_DERIV_END(t)

///////////////////////////////////////////////////////////////////////

void FuncPolyline::more_precomputations()
{
    q_.resize(nv() / 2);
    for (size_t i = 0; i < q_.size(); ++i) {
        q_[i].x = av_[2*i];
        q_[i].y = av_[2*i+1];
    }
}

CALCULATE_VALUE_BEGIN(FuncPolyline)
    realt t = get_linear_interpolation(q_, x);
CALCULATE_VALUE_END(t)

CALCULATE_DERIV_BEGIN(FuncPolyline)
    dy_dx = 0; // unused
    realt t = get_linear_interpolation(q_, x);
CALCULATE_DERIV_END(t)

///////////////////////////////////////////////////////////////////////

