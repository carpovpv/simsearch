
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <mutex>
#include <future>

#include <GraphMol/GraphMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Fingerprints/Fingerprints.h>
#include <DataStructs/BitOps.h>
#include <GraphMol/ChemTransforms/ChemTransforms.h>

#include <leveldb/db.h>
#include "ctpl.h"
#include "constants.h"
#include "clean.h"

int main( int argc , char **argv ) 
{

    if(argc != 5)
    {
	fprintf(stderr, "Usage: %s db-name smi-name number-of-threads tanimoto-cutoff.\n", argv[0]);
	return EXIT_FAILURE;
    }

    const char * dbname = argv[1];
    const char * sminame = argv[2];
    const int pool_size = atoi(argv[3]);
    const float cutoff = atof(argv[4]);

    if(pool_size <= 0)
    {
        fprintf(stderr, "Wrong number of threads was specified. Please, correct.\n");
	return EXIT_FAILURE;
    }


    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = false;

    leveldb::Status status = leveldb::DB::Open(options, dbname, &db);
    if (false == status.ok())
    {
        std::cerr << "Unable to open the database" << std::endl;
        std::cerr << status.ToString() << std::endl;

        return EXIT_FAILURE;
    }

    std::vector<ExplicitBitVect*> ds;

    char buffer[MAX_SMI_LENGTH];
    FILE * fp = fopen(sminame, "r");
    while(fgets(buffer, MAX_SMI_LENGTH, fp) != NULL)
    {
	const int len = strlen(buffer);
        if(buffer [len - 1] == '\n')	
           buffer [len - 1] = '\0';

        try
	{            
            RDKit::ROMol * mol = RDKit::SmilesToMol(buffer);
	    if(mol)
	    {
                ds.push_back(RDKit::RDKFingerprintMol(*mol));
                delete mol;
	    }
        }
        catch(...){}
    }

    printf("Number of template molecules: %ld\n", ds.size());

    unsigned long N = 0;
    std::mutex out_mutex;

    ctpl::thread_pool pool(pool_size);

    int curRes = 0;
    std::vector<std::future<void> > results(pool.size()); 

    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        std::string smiles = it->key().ToString();

        auto calcSimilarity  = [&](int threadId, const std::string &structure)
	{
	    
            try{

               RDKit::ROMol *mol = readAndClean( structure );           
	       if(mol == NULL)
		  return;

               ExplicitBitVect *descr = RDKit::RDKFingerprintMol(*mol);

               double t = 0.0;
               for(int i=0; i< ds.size(); i++)
               {
                   double a = TanimotoSimilarity(*descr, *ds[i]);
                   if(a > t)
                      t = a;
               }

               if(t > cutoff) 
	       {
		   std::lock_guard<std::mutex> guard(out_mutex);
   		   std::cout << N << " " << RDKit::MolToSmiles(*mol) << std::endl;
               }

               delete descr;           
	       delete mol;
           }
           catch(...){  }
       };// worker function 
	
       results[curRes] = pool.push(calcSimilarity, smiles);	      
       curRes++;

       if(curRes >= pool.size())
       {
	       curRes = 0;
	       for(int i=0; i< pool.size(); i++)
		       results[i].get();
       }

       N++;

    }

    assert(it->status().ok());  // Check for any errors found during the scan
    delete it;

    for(int i=0; i < ds.size(); i++)
       delete ds[i];

    printf("Total molecules screened: %ld\n", N);

    return 0;
}

