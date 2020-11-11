project(docs)
cmake_minimum_required(VERSION 3.17)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( docs docs docs.cpp 
    document_graph/document_graph.cpp 
    document_graph/edges.cpp 
    document_graph/creators.cpp 
    document_graph/accessors.cpp )
    
target_include_directories( docs PUBLIC ${CMAKE_SOURCE_DIR}/../include )
