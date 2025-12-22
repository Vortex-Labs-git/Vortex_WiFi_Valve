# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/hp/esp/v5.4.2/esp-idf/components/bootloader/subproject"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/tmp"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/src/bootloader-stamp"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/src"
  "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/hp/Documents/vortex/WiFi_valve/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
