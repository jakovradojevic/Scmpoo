source_group("Source Files" FILES Scmpoo.c)
list(APPEND SCMPOO_FILES Scmpoo.c)

if(MSVC)
  source_group("Resource Files" FILES Scmpoo.rc)
  list(APPEND SCMPOO_FILES Scmpoo.rc)
elseif(MINGW)
  source_group("Resource Files" FILES Scmpoo.mingw.rc)
  list(APPEND SCMPOO_FILES Scmpoo.mingw.rc)
endif()

add_executable(Scmpoo WIN32 ${SCMPOO_FILES})

# windres/rc only re-run when the .rc changes; also rebuild when embedded assets change.
set(SCMPOO_RC_ASSETS
  "${CMAKE_CURRENT_SOURCE_DIR}/108.wav"
  "${CMAKE_CURRENT_SOURCE_DIR}/109.wav"
  "${CMAKE_CURRENT_SOURCE_DIR}/110.wav"
  "${CMAKE_CURRENT_SOURCE_DIR}/111.wav"
)
if(MSVC)
  set_source_files_properties(Scmpoo.rc PROPERTIES OBJECT_DEPENDS "${SCMPOO_RC_ASSETS}")
elseif(MINGW)
  set_source_files_properties(Scmpoo.mingw.rc PROPERTIES OBJECT_DEPENDS "${SCMPOO_RC_ASSETS}")
endif()

if(MSVC)
  if(MSVC_VERSION GREATER_EQUAL 1900)
    target_compile_options(Scmpoo PRIVATE /source-charset:.932 /execution-charset:.932)
  endif()
  target_link_libraries(Scmpoo kernel32.lib User32.lib Gdi32.lib shell32.lib WinMM.lib ComCtl32.lib)
  set_property(TARGET Scmpoo PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET Scmpoo PROPERTY LINK_FLAGS "/level='requireAdministrator'")
  set_property(TARGET Scmpoo PROPERTY VS_DPI_AWARE "ON")
elseif(MINGW)
  target_link_libraries(Scmpoo kernel32 user32 gdi32 shell32 winmm comctl32)
  if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_custom_command(TARGET Scmpoo POST_BUILD COMMAND ${CMAKE_STRIP} -s $<TARGET_FILE:Scmpoo>)
  endif()
endif()
