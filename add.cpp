#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <limits>

#include <boost/algorithm/string.hpp>

#include <GraphMol/GraphMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Fingerprints/Fingerprints.h>
#include <DataStructs/BitOps.h>
#include <GraphMol/ChemTransforms/ChemTransforms.h>

#include <leveldb/db.h>

using namespace std;

int main(int argc, char** argv)
{

    if(argc != 3)
    {
	std::cerr << "Usage: " << argv[0] << " db-name smi-name" << std::endl;
	return EXIT_FAILURE;
    }	    

    const char * dbname = argv[1];	
    const char * sminame = argv[2];
    
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, dbname, &db);
    if (false == status.ok())
    {
        cerr << "Unable to open/create test database " << dbname << endl;
        cerr << status.ToString() << endl;

        return EXIT_FAILURE;
    }

    std::vector<RDKit::ROMol*> frags;
    frags.push_back(RDKit::SmartsToMol("[Cl,Br,I,Li,Na,K,Ca,Mg,O,N]"));
    frags.push_back(RDKit::SmartsToMol("[N](=O)(O)O"));
    frags.push_back(RDKit::SmartsToMol("[P](=O)(O)(O)O"));
    frags.push_back(RDKit::SmartsToMol("[P](F)(F)(F)(F)(F)F"));
    frags.push_back(RDKit::SmartsToMol("[S](=O)(=O)(O)O"));

    unsigned long cnt = 0;

    std::ifstream ifs;
    ifs.open (sminame, std::ifstream::in);

    leveldb::WriteOptions writeOptions;
    while(ifs.good())
    {
        cnt++;

        std::string line;
        std::getline(ifs, line);

        if (cnt == 1)
     	    continue;

        std::vector < std::string > frames;
        boost::algorithm::split(frames, line, boost::algorithm::is_any_of(" \t"));

        if (frames.size() > 1)
        {
	   std::string smile = frames[0];
           std::string id = frames[1];

	   boost::algorithm::trim(id);
	   if(id.empty())
              continue;

           std::string canonical;

	   try
	   {
              RDKit::ROMol * mol = RDKit::SmilesToMol(smile);
	      canonical = RDKit::MolToSmiles(*mol);
              delete mol;
           }
	   catch(...)
	   {

	   }

           boost::algorithm::trim(canonical);
	   if(canonical.empty()) 
              continue;

   	   db->Put(writeOptions, canonical, id);

	   if(cnt % 100000 == 0)
              std::cout << cnt << std::endl;	  
        }

    }

    delete db;

    std::cout << "Items worked out: " << cnt << std::endl;

    return EXIT_SUCCESS;
}

