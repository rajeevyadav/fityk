// This file is part of fityk program. Copyright (C) Marcin Wojdyr
// Licence: GNU General Public License ver. 2+

#ifndef FITYK__LMFIT__H__
#define FITYK__LMFIT__H__
#include "common.h"
#include <vector>
#include <map>
#include <string>
#include "fit.h"

///           Levenberg-Marquardt method
class LMfit : public Fit
{
public:
    LMfit(Ftk* F);
    ~LMfit();
    virtual void init(); // called before do_iteration()/autoiter()
    void autoiter();
private:
    std::vector<realt> alpha, alpha_;            // matrices
    std::vector<realt> beta, beta_;   // and vectors
    std::vector<realt> a;    // parameters table
    realt chi2, chi2_;
    double lambda;

    bool do_iteration();
};

#endif

