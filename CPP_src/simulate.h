#ifndef SIM_H
#define SIM_H
#include <map>
#include <string>
#include <vector>
#include "load.h"
using namespace std;
void run_sims2(map<string, vector<segment>> , vector<PSSM *> ,int  , int  , 
	int, vector<double>  , double, 
	map<int, vector<vector<double> >> &,	map<int, map<int, vector<double> >> &  );
void run_sim3(map<string, vector<segment>>, vector<PSSM *>, int,vector<vector<double>> ,vector<vector<double>>,
					vector<double> , double   );
#endif