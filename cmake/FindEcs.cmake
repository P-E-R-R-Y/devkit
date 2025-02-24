set(name ecs)

include(FetchContent)
FetchContent_GetProperties(${name}) 
if (NOT ${name}_POPULATED)
  # If not, fetch it
  FetchContent_Declare(
    ${name}
    GIT_REPOSITORY https://github.com/P-E-R-R-Y/${name}.git
    GIT_TAG main #v0.1.0  # You can replace "main" with a specific version tag or commit hash
  )
  FetchContent_MakeAvailable(${name})
endif()