################################################################################
# Doxygen Documentation
################################################################################
if(BUILD_DOC_DOXYGEN)
    # Create target for building Doxygen documentation
    add_custom_target(doc-doxygen)

    # Setup source and destination paths
    set(DOXYGEN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doxygen")
    set(DOXYGEN_BUILD_DIR  "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

    # Generate Doxygen configuration file from template by replacing specific
    # CMake variables with corresponding values. The resulting config file is
    # copied to the directory where the documentation is going to be built.
    configure_file(${DOXYGEN_SOURCE_DIR}/Doxyfile.conf.in ${DOXYGEN_BUILD_DIR}/Doxyfile.conf @ONLY)

    # Associate the command for building the documentation (i.e. "doxygen Doxfile.conf")
    # with the custom build target 'doc-doxygen'.
    add_custom_command(
        TARGET doc-doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.conf
        WORKING_DIRECTORY ${DOXYGEN_BUILD_DIR}
        COMMENT "Generating documentation with Doxygen."
        VERBATIM
    )
endif()
