project(docs)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( docs docs docs.cpp document_graph.cpp accessors.cpp )
target_include_directories( docs PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( docs ${CMAKE_SOURCE_DIR}/../ricardian )