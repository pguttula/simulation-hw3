//
// Created by KMA Solaiman on 1/23/19.
//

#ifndef SIMULATION_SSQ_H
#define SIMULATION_SSQ_H

/* External definitions for single-server queueing system */
# include <stdio.h>
# include <math.h>
# include <queue>
# include <iostream>
using namespace std;

double uniform(double *dseed) {


    double d2p31m = 2147483647,
            d2p31  = 2147483711;

    *dseed = 16807*(*dseed) - floor(16807*(*dseed)/d2p31m) * d2p31m;
    return( fabs((*dseed / d2p31)) );
}

double expon(double *dseed, float xm) {

    return( (-(xm) * log((double)uniform(dseed))) );
}


#endif //SIMULATION_SSQ_H