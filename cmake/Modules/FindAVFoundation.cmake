# Once done these will be defined:
#
#  AVFOUNDATION_FOUND
#  AVFOUNDATION_LIBRARIES

find_library(AVFOUNDATION_FRAMEWORK AVFoundation)

set(AVFOUNDATION_LIBRARIES ${AVFOUNDATION_FRAMEWORK})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AVFoundation DEFAULT_MSG AVFOUNDATION_FRAMEWORK)
mark_as_advanced(AVFOUNDATION_FRAMEWORK)
