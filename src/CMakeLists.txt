
add_executable(
  nexran
  policy.cc nodeb.cc ue.cc slice.cc restserver.cc
  config.cc nexran.cc main.cc buildinfo.cc)
target_link_libraries(nexran e2ap e2sm pistache_shared mdclog ricxfcpp rmr_si ssl crypto cpprest boost_system)
install(TARGETS nexran DESTINATION bin)
