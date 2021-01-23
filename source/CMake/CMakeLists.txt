##����CMake�汾
cmake_minimum_required(VERSION 3.13)

##������Ŀ����/����/�汾
project(QTL
    LANGUAGES CXX C
    VERSION 1.0
)


##Դ����λ��
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")


set(QT_PREFIX_PATH $ENV{QTDIR})
list(APPEND CMAKE_PREFIX_PATH ${QT_PREFIX_PATH})
message("QT install path: $ENV{QTDIR}")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PROJECT_PLATFORM "Win32")
    set(PROJECT_PLATFORM_X "X86")
    add_definitions(-DWIN32)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PROJECT_PLATFORM "x64")
    set(PROJECT_PLATFORM_X "X64")
    add_definitions(-DWIN64)
endif()


##ȫ������(���б��롢������������չ�����رվ��桢����comdat�۵�,Release�汾����PDB)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Ob2 /W0")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /OPT:REF /OPT:ICF")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
endif()

##��ֹRelWithDebInfo���Ż����򿪾���
option(BUILD_DEVELOPER "Build for developer" ON)
if(MSVC AND BUILD_DEVELOPER)
    STRING(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}) 
	STRING(REPLACE "/W0" "/W3" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif()

##���� lib·��
link_directories("${PROJECT_SOURCE_DIR}/../build/lib/${PROJECT_PLATFORM}/$(Configuration)")

##����VS��������ʹ��"�ļ���"��֯�ṹ
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##����̬��\��̬�����·����λ���ض�·��,������ʱʹ��(���������Ŀ�ֲ��ڸ����ļ���)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}../build/bin/${PROJECT_PLATFORM}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}../build/lib/${PROJECT_PLATFORM}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}../build/lib/${PROJECT_PLATFORM}/$<CONFIG>")
#message(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})


# �����İ���Ҫ��ǰ��
find_package(Qt5Core REQUIRED )
find_package(Qt5Gui REQUIRED )
find_package(Qt5Widgets REQUIRED )


add_subdirectory(SharedMemory)


# ����������
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT SharedMemory)