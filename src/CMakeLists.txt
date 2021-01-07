
add_executable(nexran policy.cc nodeb.cc ue.cc slice.cc restserver.cc nexran.cc main.cc buildinfo.cc)
target_link_libraries(nexran pistache_shared)
install(TARGETS nexran DESTINATION bin)
