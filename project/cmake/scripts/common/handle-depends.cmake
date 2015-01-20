function(download_project project_id project_definition target_dir)
  # get the URL and revision of the addon
  list(LENGTH project_definition deflength)
  list(GET project_definition 0 id)
  list(GET project_definition 1 url)

  set(project_download_file "${id}")
  # download and extract all addons
  if(deflength GREATER 2)
    # if there is a 3rd parameter in the file, we consider it a git revision
    list(GET project_definition 2 revision)

    # Note: downloading specific revisions via http in the format below is probably github specific
    # if we ever use other repositories, this might need adapting
    set(url ${url}/archive/${revision}.tar.gz)

    set(project_download_file "${project_download_file}-${revision}")
  endif()
  if(NOT EXISTS ${target_dir}/download/${project_id}.tar.gz)
    file(DOWNLOAD "${url}" "${target_dir}/download/${project_id}.tar.gz" STATUS dlstatus LOG dllog SHOW_PROGRESS)
    list(GET dlstatus 0 retcode)
    if(NOT ${retcode} EQUAL 0)
      message(FATAL_ERROR "ERROR downloading ${url} - status: ${dlstatus} log: ${dllog}")
    endif()
  endif()
  if(EXISTS "${target_dir}/${project_download_file}")
    file(REMOVE_RECURSE "${target_dir}/${project_download_file}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzvf ${target_dir}/download/${project_id}.tar.gz
                  WORKING_DIRECTORY ${target_dir})
  file(GLOB extract_dir "${target_dir}/${project_download_file}*")
  if(extract_dir STREQUAL "")
    message(FATAL_ERROR "Error extracting ${target_dir}/download/${project_id}.tar.gz")
  else()
    file(RENAME "${extract_dir}" "${target_dir}/${project_id}")
  endif()
endfunction()

function(get_dependency_id dependency_file dependency_id)
  file(STRINGS ${dependency_file} def)
  separate_arguments(def)
  list(LENGTH def deflength)

  if(NOT "${def}" STREQUAL "")
    # read the id and the url from the file
    list(GET def 0 id)

    # get the revision of the dependency
    if (deflength GREATER 2)
      list(GET def 2 revision)
      if(NOT "${revision}" STREQUAL "")
        set(id "${id}-${revision}")
      endif()
    endif()
  else()
    # read the id from the filename
    get_filename_component(id ${dependency_file} NAME_WE)
  endif()

  set(${dependency_id} "${id}" PARENT_SCOPE)
endfunction()

# handle addon/dependency depends
function(add_project_depends project_id searchpath)
  # input: string project_id string searchpath

  set(OUTPUT_DIR ${DEPENDS_PATH})
  file(GLOB_RECURSE cmake_input_files ${searchpath}/${CORE_SYSTEM_NAME}/*.txt)
  file(GLOB_RECURSE cmake_input_files2 ${searchpath}/common/*.txt)
  list(APPEND cmake_input_files ${cmake_input_files2})

  foreach(file ${cmake_input_files})
    if(NOT (file MATCHES CMakeLists.txt OR
            file MATCHES install.txt OR
            file MATCHES noinstall.txt OR
            file MATCHES flags.txt OR
            file MATCHES deps.txt))
      message(STATUS "Processing ${file}")

      # get the id of the dependency
      get_dependency_id("${file}" dependency_id)
      message(STATUS "Dependency ID: ${dependency_id}")

      list(FIND downloaded_depends ${file} idx)
      if(idx LESS 0 AND NOT TARGET ${dependency_id})
        message(STATUS "Downloading ${dependency_id}")

        # read the dependency definition from the file
        file(STRINGS ${file} def)
        separate_arguments(def)

        # download the dependency's project settings and source code
        download_project(${dependency_id} "${def}" "${BUILD_DIR}")

        list(APPEND downloaded_depends ${file})
      elseif(TARGET ${dependency_id})
         message(STATUS "${dependency_id} already built")
      else()
         message(STATUS "${dependency_id} already downloaded")
      endif()
    endif()
  endforeach()

  foreach(dependency_file ${downloaded_depends})
    # get the id of the dependency
    get_dependency_id("${dependency_file}" id)

    message(STATUS "Preparing ${id}")

    # get the directory path of the dependency file
    get_filename_component(dir ${dependency_file} PATH)

    # check if there are any library specific flags that need to be passed on
    if(EXISTS ${dir}/flags.txt)
      file(STRINGS ${dir}/flags.txt extraflags)
      separate_arguments(extraflags)
      message(STATUS "${id} extraflags: ${extraflags}")
    endif()

    set(BUILD_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
                   -DOUTPUT_DIR=${OUTPUT_DIR}
                   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                   -DCMAKE_USER_MAKE_RULES_OVERRIDE=${CMAKE_USER_MAKE_RULES_OVERRIDE}
                   -DCMAKE_USER_MAKE_RULES_OVERRIDE_CXX=${CMAKE_USER_MAKE_RULES_OVERRIDE_CXX}
                   -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
                   -DARCH_DEFINES=${ARCH_DEFINES}
                   -DENABLE_STATIC=1
                   -DBUILD_SHARED_LIBS=0)

    if(CMAKE_TOOLCHAIN_FILE)
      list(APPEND BUILD_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE})
      message(STATUS "${id}: toolchain specified")
    endif()

    # if there's a CMakeLists.txt use it to prepare the build
    if(EXISTS ${dir}/CMakeLists.txt)
      message(STATUS "${id} use custom CMakeLists.txt")
      file(APPEND ${BUILD_DIR}/${id}/tmp/patch.cmake
           "file(COPY ${dir}/CMakeLists.txt
               DESTINATION ${BUILD_DIR}/${id})\n")
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${BUILD_DIR}/${id}/tmp/patch.cmake)
    else()
      set(PATCH_COMMAND "")
    endif()

    # check if we have patches to apply
    file(GLOB patches ${dir}/*.patch)
    list(SORT patches)
    foreach(patch ${patches})
      set(PATCH_COMMAND ${CMAKE_COMMAND} -P ${BUILD_DIR}/${id}/tmp/patch.cmake)
      file(APPEND ${BUILD_DIR}/${id}/tmp/patch.cmake
           "execute_process(COMMAND patch -p1 -i ${patch})\n")
    endforeach()

    # if there's an install.txt use it to properly install the built files
    if(EXISTS ${dir}/install.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND ${CMAKE_COMMAND}
                                          -DINPUTDIR=${BUILD_DIR}/${id}/
                                          -DINPUTFILE=${dir}/install.txt
                                          -DDESTDIR=${OUTPUT_DIR}
                                          -DENABLE_STATIC=1
                                          "${extraflags}"
                                          -P ${PROJECT_SOURCE_DIR}/install.cmake)
    elseif(EXISTS ${dir}/noinstall.txt)
      set(INSTALL_COMMAND INSTALL_COMMAND "")
    else()
      set(INSTALL_COMMAND "")
    endif()

    # check if there's a deps.txt containing dependencies on other libraries
    if(EXISTS ${dir}/deps.txt)
      file(STRINGS ${dir}/deps.txt ${id}_DEPS)
    endif()

    # prepare the setup of the call to externalproject_add()
    set(EXTERNALPROJECT_SETUP CMAKE_ARGS ${extraflags} ${BUILD_ARGS}
                              PATCH_COMMAND ${PATCH_COMMAND}
                              ${INSTALL_COMMAND})

  ##            if(WIN32)
  ##              set(CONFIGURE_COMMAND "")
  ##            else()
  ##              set(CONFIGURE_COMMAND PKG_CONFIG_PATH=${OUTPUT_DIR}/lib/pkgconfig
  ##                                    ${CMAKE_COMMAND} -DCMAKE_LIBRARY_PATH=${OUTPUT_DIR}/lib ${extraflags}
  ##                                    ${BUILD_DIR}/${id}/src/${id}
  ##                                    -DPACKAGE_CONFIG_PATH=${OUTPUT_DIR}/lib/pkgconfig
  ##                                    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  ##                                    -DOUTPUT_DIR=${OUTPUT_DIR}
  ##                                    -DCMAKE_PREFIX_PATH=${OUTPUT_DIR}
  ##                                    -DCMAKE_INSTALL_PREFIX=${OUTPUT_DIR}
  ##                                    -DCMAKE_EXE_LINKER_FLAGS=-L${OUTPUT_DIR}/lib
  ##                                    -DCMAKE_INCLUDE_PATH=${OUTPUT_DIR}/include)
  ##            endif()
  ##
  ##            externalproject_add(${id}
  ##                                URL ${url}
  ##                                DOWNLOAD_DIR ${BUILD_DIR}/download
  ##                                CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
  ##                                ${EXTERNALPROJECT_SETUP})

    externalproject_add(${id}
                        SOURCE_DIR ${BUILD_DIR}/${id}
                        INSTALL_DIR ${OUTPUT_DIR}
                        ${EXTERNALPROJECT_SETUP})

    set(${id}_DEPENDS_DIR ${BUILD_DIR}/${id}/depends)

    # handle dependencies of the dependency
    if(EXISTS ${${id}_DEPENDS_DIR})
      add_project_depends(${id} ${${id}_DEPENDS_DIR})
    endif()

    # add any discovered dependencies
    if (${id}_DEPS AND NOT "${${id}_DEPS}" STREQUAL "")
      message(STATUS "${id} dependencies: ${${id}_DEPS}")
      add_dependencies(${id} ${${id}_DEPS})
    endif()

    set(${project_id}_DEPS ${${project_id}_DEPS} ${id})
    set(${project_id}_DEPS "${${project_id}_DEPS}" PARENT_SCOPE)
  endforeach()
endfunction()

