project(src_p)

file(GLOB helloworld_SRC
    "*.h"
    "*.cpp"
)

add_executable(src_p ${helloworld_SRC})