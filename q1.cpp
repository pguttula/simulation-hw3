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
    cerr << "  Usage: ./q1 lambda mu0 mu1 prob0 seed" << endl;
    cerr << "  Example: ./q1 1.0 6.0 5.0 0.2 123457" << endl;
}

int main(int argc,char* argv[]) {

    // read the arguments from command line
    if (argc < 6) {
        printUsage();
        exit(1);
    } else{
        // argv[0] is program name
        double lambda = stof(argv[1]);
        double mu0 = stof(argv[2]);
        double mu1 = stof(argv[3]);
        double prob0 = stof(argv[4]);
        seed = stof(argv[5]);

        cout << seed << endl;

        // TODO: Initialize arrival and service means, statistics variables

        // TODO: Simulation Process

        // TODO: Output Statistics

    }

    return 0;
}