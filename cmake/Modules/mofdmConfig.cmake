INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_MOFDM mofdm)

FIND_PATH(
    MOFDM_INCLUDE_DIRS
    NAMES mofdm/api.h
    HINTS $ENV{MOFDM_DIR}/include
        ${PC_MOFDM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREEFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    MOFDM_LIBRARIES
    NAMES gnuradio-mofdm
    HINTS $ENV{MOFDM_DIR}/lib
        ${PC_MOFDM_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MOFDM DEFAULT_MSG MOFDM_LIBRARIES MOFDM_INCLUDE_DIRS)
MARK_AS_ADVANCED(MOFDM_LIBRARIES MOFDM_INCLUDE_DIRS)

