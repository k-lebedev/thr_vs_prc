add_executable(thr_vs_prc main.cpp thread_worker.cpp process_worker.cpp)
target_compile_options(thr_vs_prc PRIVATE -Wall -Wextra -Wconversion -Wshadow)
target_link_libraries(thr_vs_prc sub::cos_log pthread)