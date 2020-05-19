cmake_minimum_required(VERSION 3.10)
project(fost-postgres-circleci)

# Opt in to the tests we want to run
add_custom_target(check)
add_custom_target(pgtest)

add_subdirectory(boost)
add_subdirectory(crypto)
add_subdirectory(cord)
add_subdirectory(fost-base)
add_subdirectory(.. fost-postgres)
add_subdirectory(libpqxx)
add_subdirectory(threading)
