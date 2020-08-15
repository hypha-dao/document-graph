project(document)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( document document document.cpp )
target_include_directories( document PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( document ${CMAKE_SOURCE_DIR}/../ricardian )