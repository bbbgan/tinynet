set(base_SourceFile
  Logger.cc
  LogFile.cc
  AsyncLogger.cc
  )

add_library(tinynet_base ${base_SourceFile})
target_link_libraries(tinynet_base)
install(TARGETS tinynet_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/tinynet/base)

