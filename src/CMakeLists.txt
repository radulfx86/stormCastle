set(stormCastle ./main.cpp)

file(GLOB SOURCES
    *.h
    *.cpp
	)

add_executable(stormCastle ${SOURCES} main.cpp)


#include_directories(utils
#					common
#					external
#					display
#                    .)
