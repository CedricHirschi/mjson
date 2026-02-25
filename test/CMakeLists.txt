# Test configuration for mjson
add_executable(unit_test
    unit_test.c
)

target_link_libraries(unit_test PRIVATE mjson)

# Add test definitions that are in the Makefile
target_compile_definitions(unit_test PRIVATE
    MJSON_ENABLE_MERGE=1
    MJSON_ENABLE_PRETTY=1
)

# Add the test to CTest
add_test(NAME mjson_unit_test COMMAND unit_test)
