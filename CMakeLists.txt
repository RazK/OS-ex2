# CMake instructions to make the static lib
ADD_LIBRARY( uthreads STATIC uthreads.cpp )

# CMake instructions to test using the static lib
SET( EXE_1 StaticTest )
SET( EXE_2 DemoItimer )
SET( EXE_3 DemoJmp )
SET( EXE_4 DemoSingIntHandler )

ADD_EXECUTABLE( ${EXE_1} main.cpp )
ADD_EXECUTABLE( ${EXE_2} demo_itimer.c uthread.cpp)
ADD_EXECUTABLE( ${EXE_3} demo_jmp.c )
ADD_EXECUTABLE( ${EXE_4} demo_singInt_handler.c)

TARGET_LINK_LIBRARIES( ${EXE_1} uthreads )
TARGET_LINK_LIBRARIES( ${EXE_2} uthreads )
TARGET_LINK_LIBRARIES( ${EXE_3} uthreads )
TARGET_LINK_LIBRARIES( ${EXE_4} uthreads )