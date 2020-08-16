# Once done these will be defined:
#
#  IOKIT_FOUND
#  IOKIT_LIBRARIES

find_library(IOKIT_FRAMEWORK IOKit)

set(IOKIT_LIBRARIES ${IOKIT_FRAMEWORK})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IOKit DEFAULT_MSG IOKIT_FRAMEWORK)
mark_as_advanced(IOKIT_FRAMEWORK)
