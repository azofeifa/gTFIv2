DB=/Users/joazofeifa/Lab/gTFIv2/PSSM_DB/new_motif_db_format.txt
DB2=/Users/joazofeifa/Desktop/generated_DB.txt
fasta=~/Lab/genome_files/hg19.fa
bed=~/Lab/gro_seq_files/Allen2014/EMG_out_files/test_Allen2014_DMSO2_3-19_divergent_classifications.bed
out=/Users/joazofeifa/Desktop/generated_DB.txt
out2=/Users/joazofeifa/Desktop/stats.txt
br=100
pv=0.00001
log_out=/Users/joazofeifa/Lab/gro_seq_files/Allen2014/EMG_out_files/
ID=test
simN=10000
bed_out=/Users/joazofeifa/Lab/motif_distances/test_bed_motif_files/
NP=1
site_br=10
boot_strap_number=100
test=1
#mpirun -np $NP ./SE DB -DB $DB -fasta $fasta -bed $bed -o $out -sim_N $simN -br $br -pv $pv

mpirun -np $NP ./SE EVAL -DB $DB2 -fasta $fasta -bed $bed -o $out2 -bsn $boot_strap_number