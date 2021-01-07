#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <limits>

#include <boost/algorithm/string.hpp>
#include <leveldb/db.h>

#include <GraphMol/GraphMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>

#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>

#include <string.h>
#include "constants.h"
#include "clean.h"

using namespace std;

volatile bool HALT = false;

int server(const char * dbname, int port)
{

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

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
	perror("socket failed");
	return EXIT_FAILURE;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		   &opt, sizeof(opt))) 
    {
	perror("setsockopt");
	return EXIT_FAILURE;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) 
    {
	perror("bind failed");
	return EXIT_FAILURE;
    }

    if (listen(server_fd, 3) < 0) 
    {
	perror("listen");
	return EXIT_FAILURE;
    }

    while (!HALT) 
    {
	if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
				 (socklen_t *) & addrlen)) < 0) 
	{
	    perror("accept");
 	    continue;	    
	}
       
	std::string req;
	while(true)
	{ 
	   char d[MAX_SMI_LENGTH];
	   int rcnt = read(new_socket, d, MAX_SMI_LENGTH);
	   if(rcnt >= 0) 
	   {
               d[rcnt] = '\0';	       
	       char * p = strchr(d, '\n');
	       if(p != NULL)
	       {
		   *p = '\0';
                   req += d;

		   break;
	       }
	       else 
	    	   req += d;
	   }
	   else 
    	       break;          
	}      

	printf("Request: %s\n", req.c_str());
 	std::string canonical;
        try
        {
            RDKit::ROMol * mol = readAndClean(req);
	    if(mol)
	    {
               canonical = RDKit::MolToSmiles(*mol);
               delete mol;
	    }
        }
        catch(...)
        {

        }

        boost::algorithm::trim(canonical);
        if(canonical.empty()) 
            write(new_socket, "none\n", 5);
	else 
	{
            std::string value;
            leveldb::Status s = db->Get(leveldb::ReadOptions(), canonical, &value);
            
	    if (s.ok()) 
	    {
		value += '\n';
		write(new_socket, value.c_str(), value.size());
	    }
	    else 
   	        write(new_socket, "none\n", 5);
	}

        close(new_socket);
    } 
    close(server_fd);
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        std::cerr << "Usage: %s db-name port-number." << argv[0] << std::endl;
	return EXIT_FAILURE;
    }	    

    const char * dbname = argv[1];
    const int port = atoi(argv[2]);

    return server(dbname, port);
}

