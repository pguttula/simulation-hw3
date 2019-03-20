/* External definitions for single-server queueing system */
# include <stdio.h>
# include <cmath>
# include <queue>
#include <ctime>
#include <fstream>
#include <string>
# include <iostream>
using namespace std;
# include "ssq.h"

/* 5. Random Number Generator */
double uniform(double *);
double expon(double *, float);

double seed; //set the seed only once, at the start.


void printUsage() {
    cerr << "  Usage: ./q2 lambda1 lambda2 mu seed" << endl;
    cerr << "  Example: ./q2 0.33 0.25 0.5 123457" << endl;
}

int main(int argc,char* argv[]) {

    // read the arguments from command line
    if (argc < 5) {
        printUsage();
        exit(1);
    } else{
        // argv[0] is program name
        double lambda1 = stof(argv[1]);
        double lambda2 = stof(argv[2]);
        double mu = stof(argv[3]);
        seed = stof(argv[4]);

        cout << lambda1 << endl;

        // TODO: Initialize arrival and service means, statistics variables

        // TODO: Simulation Process

        // TODO: Output Statistics

    }

    return 0;
}