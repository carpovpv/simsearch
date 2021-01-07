set(RDKIT_DIR $ENV{RDBASE})

set(RDKIT_INCLUDE_DIR ${RDKIT_DIR}/Code)
set(RDKIT_FOUND ${RDKIT_DIR})

# libraries, as specified in the COMPONENTS
foreach(component ${RDKit_FIND_COMPONENTS})
  message( "Looking for RDKit component ${component}" )
  find_file( RDKit_LIBRARY_${component}
    libRDKit${component}.so
    PATH ${RDKIT_DIR}/lib NO_DEFAULT_PATH)
  message("RDKit_LIBRARY_${component} : ${RDKit_LIBRARY_${component}}")
  if(${RDKit_LIBRARY_${component}} MATCHES "-NOTFOUND$")
    message(FATAL_ERROR "Didn't find RDKit ${component} library.")
  endif(${RDKit_LIBRARY_${component}} MATCHES "-NOTFOUND$")
  set(RDKIT_LIBRARIES ${RDKIT_LIBRARIES} ${RDKit_LIBRARY_${component}})
endforeach(component)

message("RDKIT_INCLUDE_DIR : ${RDKIT_INCLUDE_DIR}")
message("RDKIT_LIBRARIES : ${RDKIT_LIBRARIES}")
message("RDKIT_FOUND : ${RDKIT_FOUND}")

