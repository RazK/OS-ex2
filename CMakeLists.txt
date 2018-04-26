# CMake instructions to make the static lib

ADD_LIBRARY( uthreads STATIC
        uthreads.cpp )


# CMake instructions to test using the static lib

SET( APP_EXE StaticTest )

ADD_EXECUTABLE( ${APP_EXE}
        main.cpp )

TARGET_LINK_LIBRARIES( ${APP_EXE}
        uthreads )