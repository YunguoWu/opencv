#message("Load TOOLCHAIN config...")
set(CMAKE_CXX_COMPILER /opt/imgtec/Toolchains/mips-mti-linux-gnu/2018.09-03/bin/mips-mti-linux-gnu-g++)
set(CMAKE_CXX_FLAGS "-march=mips32r5 -EL -mmsa -mhard-float -mfp64 -mnan=2008 -mabs=2008 -O3 -ffp-contract=off -static")
set(CMAKE_C_COMPILER /opt/imgtec/Toolchains/mips-mti-linux-gnu/2018.09-03/bin/mips-mti-linux-gnu-gcc)
set(CMAKE_C_FLAGS "-march=mips32r5 -EL -mmsa -mhard-float -mfp64 -mnan=2008 -mabs=2008 -O3 -ffp-contract=off -static")
set(CMAKE_EXE_LINKER_FLAGS "-lpthread -lrt -ldl -Bstatic")
set(CMAKE_FIND_ROOT_PATH /usr/bin)
set(CMAKE_MODULE_LINKER_FLAGS )
set(CMAKE_SHARED_LINKER_FLAGS )
set(CMAKE_SKIP_RPATH ON)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips32r5el)
set(CMAKE_SYSTEM_VERSION 1)
set(GCC_COMPILER_VERSION )
set(MIPS_LINUX_SYSROOT /usr/bin)
