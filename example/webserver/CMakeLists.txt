set(http_SourceFile
  HttpContext.cc
  HttpResponse.cc
  HttpServer.cc
  HttpContext.cc
  )

add_library(tinyhttp ${http_SourceFile})
target_link_libraries(tinyhttp)

add_executable(tinywebserver Webserver.cc)
target_link_libraries(tinywebserver tinyhttp tinynet_base tinynet_net)