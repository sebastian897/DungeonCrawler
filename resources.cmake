# finds .png resources in ./resources
# generates c src code with the byte conetnt using xxd
# configures resources.c and resources.h from their .in templates
# provides GEN_C_FILES for inclusion in project

function(generate_resources PNG_LIST GEN_DIR OUT_C_FILES
    OUT_RESOURCE_ENUMS OUT_EXTERN_DECLS OUT_RESOURCE_ENTRIES)

  find_program(XXD_EXE xxd REQUIRED)
  find_program(SED_EXE sed REQUIRED)
  file(MAKE_DIRECTORY ${GEN_DIR})

  set(resource_enum_list "")
  set(extern_list "")
  set(resource_entry_list "")
  set(gen_c_files_list "")

  foreach(FILE ${PNG_LIST})
    get_filename_component(NAME ${FILE} NAME_WE)
    string(TOUPPER ${NAME} ID)
    string(REPLACE "." "_" VAR ${NAME})
    string(PREPEND VAR "resource_")
    file(SIZE ${FILE} SIZE)

    set(OUT_C ${GEN_DIR}/${VAR}.c)
    add_custom_command(
      OUTPUT ${OUT_C}
      COMMAND ${XXD_EXE} -i -n ${VAR} ${FILE} > ${OUT_C}
        # make array const and remove size (covered below)
        # ${SED_EXE} -e 's/^unsigned char/const unsigned char/ \; /^unsigned int/d' > ${OUT_C}
      DEPENDS ${FILE} # only if new
    )
    list(APPEND gen_c_files_list ${OUT_C})

    # Accumulate lists for template substitution
    list(APPEND resource_enum_list "  RES_${ID},")
    list(APPEND extern_list "extern const unsigned char ${VAR}[]\;")
    list(APPEND resource_entry_list "  [RES_${ID}] = {${VAR}, ${SIZE}},")
  endforeach()

  string(JOIN "\n" RESOURCE_ENUMS ${resource_enum_list})
  string(JOIN "\n" EXTERN_DECLARATIONS ${extern_list})
  string(JOIN "\n" RESOURCE_ENTRIES ${resource_entry_list})

  set(${OUT_C_FILES} "${gen_c_files_list}" PARENT_SCOPE)
  set(${OUT_RESOURCE_ENUMS} "${RESOURCE_ENUMS}" PARENT_SCOPE)
  set(${OUT_EXTERN_DECLS} "${EXTERN_DECLARATIONS}" PARENT_SCOPE)
  set(${OUT_RESOURCE_ENTRIES} "${RESOURCE_ENTRIES}" PARENT_SCOPE)
endfunction()

file(GLOB PNG_FILES resources/*.png)
set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)

generate_resources("${PNG_FILES}" "${GEN_DIR}"
  GEN_C_FILES
  RESOURCE_ENUMS
  EXTERN_DECLARATIONS
  RESOURCE_ENTRIES
)

configure_file(resources.h.in ${GEN_DIR}/resources.h @ONLY) # uses RESOURCE_ENUMS
configure_file(resources.c.in ${GEN_DIR}/resources.c @ONLY) # uses EXTERN_DECLARATIONS, RESOURCE_ENTRIES

list(APPEND GEN_C_FILES ${GEN_DIR}/resources.c)
