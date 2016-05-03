#include "scanner.h"
#include <iostream>
#include <fstream>
#include "collect_sample_statistics.h"
#include <cmath>
#include <omp.h>
#include <mpi.h>

using namespace std;

vector<double> get_GC_content(map<string, vector<segment>> S){
	vector<double> ATGC = {1,1,1,1};
	map<char, int> table;
	table['A'] 	= 0, table['C']=1, table['G']=2, table['T']=3;
	table['a'] 	= 0, table['c']=1, table['g']=2, table['t']=3;
	
	string white_space 	= "";
	for (int i = 0 ; i < S.size(); i++){
		white_space+= " ";
	}
	int counter=0;
	string stars 	= "";

	typedef map<string, vector<segment>>::iterator it_type; 
	for (it_type c = S.begin(); c!=S.end(); c++){
		
		for (int i = 0 ; i < c->second.size(); i++ ){
			string seq 	= c->second[i].seq;
			for (int j = 0 ; j < seq.size(); j++){
				if (table.find(seq[j]) != table.end()){
					ATGC[table[seq[j]]]+=1;
				}
			}
		}
		stars+="*";
		counter+=1;
	}
	double SUM 	= 0;
	for (int i = 0 ; i < ATGC.size(); i++){
		SUM+=ATGC[i];
	}
	for (int i = 0 ; i < ATGC.size(); i++){
		ATGC[i]/=SUM;
	}
	return ATGC;
}


vector<int> get_sig_positions(int forward[2000], 
	int reverse[2000], int N, PSSM * p, vector<double> background, double pv){

	vector<int> locs_pvs;
	int length 	= p->frequency_table.size();
	double pvaluef, pvaluer,llr,llf;
	int k,j;
	bool collect;
	int BOUND 	= N-length;
	int i;
	int f, r, l;
	for (i =0 ; i  < BOUND; i++){
		llf 	= 0;
		llr 	= 0;
		k 		= 0;
		j 		= i;
		l 		= i + length-1;
		for (k=0; k < length; k++){
			f 	= forward[j], r 	= reverse[l];
			llf+= (p->frequency_table[k][f])  ;
			llr+= (p->frequency_table[k][r]) ;
			j++;
			l--;
		}
		pvaluef 	= 1.0-p->get_pvalue(llf*2);
		pvaluer 	= 1.0-p->get_pvalue(llr*2);
		if (pvaluef < pv){
			locs_pvs.push_back(i);
			
		}
		if (pvaluer < pv){
			locs_pvs.push_back(i);
		}
	}
	return locs_pvs;
}



vector<segment> collapse_map(map<string, vector<segment>> S){
	typedef map<string, vector<segment>>::iterator it_type; 
	vector<segment> DD ;	
	for (it_type c = S.begin(); c!=S.end(); c++){
		for (int i = 0 ; i < c->second.size(); i++){
			DD.push_back(c->second[i]);
		}
	}
	return DD;
}
vector<int> to_vector_2(int * array, int S){
	vector<int> A(S);
	for (int s = 0 ; s < S; s++){
		A[s] 	= array[s];
	}
	return A;
}
void fill_array(vector<int> DD, int * A){
	for (int i = 0; i < DD.size(); i++){
		A[i] 	= DD[i];
	}
}


void send_out_displacement_data(vector<int> & DD, int rank, 
										int nprocs){
	if (rank==0){
		for (int j = 1 ; j < nprocs; j++){
			int S;
			MPI_Recv(&S, 1, MPI_INT, j, 1, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if (S>0){
				int * A = new int[S];
				MPI_Recv(&A[0], S, MPI_INT, j, 2, MPI_COMM_WORLD,MPI_STATUS_IGNORE);					
				vector<int> temp 	= to_vector_2(A, S);
				DD.insert(DD.end(), temp.begin(), temp.end());
			}
		}
	}else{
		int S 	= DD.size();
		MPI_Ssend(&S, 1, MPI_INT, 0,1, MPI_COMM_WORLD);
		if (S>0){
			int * A = new int[S];
			fill_array(DD, A);
			MPI_Ssend(&A[0], S, MPI_INT, 0,2, MPI_COMM_WORLD);
		}
	}
}

string get_dots_2(int N){
	string line="";
	for (int i = 0 ; i < N; i++){
		line+=".";
	}
	return line;
}


void scan_intervals(map<string, vector<segment>> S ,
 												vector<PSSM *> PSSMS, vector<double> background, 
 												double pv, int interval_size, int bsn, 
 												int rank, int nprocs, Log_File * LG){

	vector<segment> D 	= collapse_map(S);
	int threads  		= omp_get_max_threads();
	int diff 			= D.size() / nprocs;
	int start 	= rank*diff, stop 	= min(int( (rank+1)*diff),int(D.size()));
	if (rank+1==nprocs){
		stop 	= D.size();
	}
	clock_t t;

	for (int p = 0 ; p < PSSMS.size(); p++){
		vector<vector<int>> displacements(int(stop-start));
		int l 	= 0;
		int WN 	= max(int(44 - PSSMS[p]->name.size()), 1);	
		t = clock();
		LG->write(to_string(D.size()) + "\n",1);
		LG->write(PSSMS[p]->name + get_dots_2(WN), 1);
		#pragma omp parallel for
		for (int i = start ; i < stop; i++ ){
			LG->write(to_string(i) + "\t" + to_string(D.size()) + "\t" + to_string(PSSMS[p]->frequency_table.size()) + "\t" + to_string(sizeof(D[i].forward)) + "\t" + to_string(sizeof(D[i].reverse)) + "\n",1);
			displacements[l] 	= get_sig_positions(D[i].forward, D[i].reverse, 2000, PSSMS[p], background, pv);
			l++;
		}
		LG->write("finished scanning...", 1);
		vector<int> final_displacements;
		for (int i =0 ; i < displacements.size(); i++){
			for (int j = 0 ; j < displacements[i].size(); j++ ){
				final_displacements.push_back(displacements[i][j]);
			}
		}
		LG->write("finished inserting...", 1);
		LG->write("MPI...", 1);
		send_out_displacement_data(final_displacements, rank, nprocs);
		LG->write("MPI DONE...", 1);
		t = clock() - t;
		LG->write("done: " + to_string(float(t)/(CLOCKS_PER_SEC*threads)) + " seconds (" + to_string(p+1) + "/" + to_string(PSSMS.size())+")\n", 1);
		if (rank==0){
			double MD_score 		= get_MD_score(final_displacements,100,true);
			double ENRICH_score 	= get_MD_score(final_displacements,100,false);
			double NN 				= final_displacements.size();
			PSSMS[p]->MD_score 		= MD_score;
			PSSMS[p]->ENRICH_score 	= ENRICH_score;		
			PSSMS[p]->total 		= NN;
			build_cdfs_PSSMs(PSSMS[p], bsn, interval_size, NN);
			PSSMS[p]->get_pvalue_stats();
		}
	}



}



























