
#include "clean.h"

std::vector<RDKit::ROMol*> getFrags()
{
    std::vector<RDKit::ROMol*> frags;
    frags.push_back(RDKit::SmartsToMol("[Cl,Br,I,Li,Na,K,Ca,Mg,O,N]"));
    frags.push_back(RDKit::SmartsToMol("[N](=O)(O)O"));
    frags.push_back(RDKit::SmartsToMol("[P](=O)(O)(O)O"));
    frags.push_back(RDKit::SmartsToMol("[P](F)(F)(F)(F)(F)F"));
    frags.push_back(RDKit::SmartsToMol("[S](=O)(=O)(O)O"));

    return frags;
}

RDKit::ROMol * readAndClean(const std::string &smiles)
{
    static std::vector<RDKit::ROMol*> frags = getFrags();

    RDKit::ROMol * mol = RDKit::SmilesToMol(smiles);
    if(mol == NULL)
        return NULL;

    for(int i=0; i< frags.size(); i++)
    {
        RDKit::ROMol *m = RDKit::deleteSubstructs(*mol, *frags[i], true);
        delete mol;
        mol = m;
    }

    return mol;
}
