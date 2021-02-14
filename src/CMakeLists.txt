project(docs)
cmake_minimum_required(VERSION 3.17)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( docs docs 
    docs.cpp 
    document_graph/util.cpp
    document_graph/content.cpp
    document_graph/content_wrapper.cpp
    document_graph/document.cpp
    document_graph/document_graph.cpp 
    document_graph/edge.cpp )
    
target_include_directories( docs PUBLIC ${CMAKE_SOURCE_DIR}/../include )
