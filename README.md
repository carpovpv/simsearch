# Similarity search
Multi-threaded similarity search in LevelDb databases. 

## Motivation
In early-stage drug-development, it is important to find equal or similar to hits structures in big databases (open-access or in-house). For example, the REAL database contains more than 1 Billion compounds. To find out if a compound is in this database, the LevelDB index is used. The canonical SMILES is a key, and the ID of a compound is a value. Thus, the search is very fast and as a result, we also have a link to a web page of the found compound. The LevelDBServ provides a TCP-server that searches an input query in the index and returns ID if the query is found, or none, otherwise. 

## Dependency
* LevelDB (https://github.com/google/leveldb)
* RDKit (https://github.com/rdkit/rdkit)

## Build
The project uses CMake build system.

## Usage
The project consists of three programs: LevelDbAdd - to create an index; LevelDbSearch - to search similar compounds; LevelDbServ - a TCP-server for on-line exact search.

### LevelDbAdd - creating an index

LevelDbAdd program accepts two arguments: the name of the LevelDB database (if the DB is missing, it will be created) and the file with compounds. The file is treated as space/tab delimited string, where the first item in a line (zero position) corresponds to a SMILES string, the second item (1st position) - ID of the compound. Other items may also be present but not important. Before adding to the index, the SMILES is canonicalized.

### LevelDbSearch - search similar compounds

LevelDbSearch has the following arguments:

1. LevelDb database name (haystack),
2. File name with SMILES (needle),
3. Number of threads to be used,
4. Tanimoto cutoff value, usually set to 0.75 for drug-like compounds.

Found compounds are printed to stdout.

### LevelDbServ - a TCP server

LevelDbServ has two arguments: the LevelDb file name and the port to start the TCP-server. 
The server assumes receiving lines (separated with '\n', Linux style). Each line is converted to a canonical SMILES and searched in the index. If found, the ID is returned, otherwise none.



