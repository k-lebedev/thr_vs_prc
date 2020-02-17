add_executable(thr_vs_prc main.cpp thread_worker.cpp process_worker.cpp mtx.cpp)
target_compile_options(thr_vs_prc PRIVATE -Wall -Wextra -Wconversion -Wshadow)
target_link_libraries(thr_vs_prc sub::cos_log pthread)
set_target_properties(thr_vs_prc
                      PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if (USE_SYSV_SHM)
    target_compile_definitions(thr_vs_prc PRIVATE -DUSE_SYSV_SHM=1)
endif(USE_SYSV_SHM)