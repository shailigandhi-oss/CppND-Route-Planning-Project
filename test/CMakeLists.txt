
add_executable(test a_search_unittest.cpp)
target_link_libraries(test gtest_main route_planner pugixml)
add_test(NAME test COMMAND test)