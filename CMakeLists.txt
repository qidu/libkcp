cmake_minimum_required(VERSION 2.8.7)
project(qtp)

set(LIB_SOURCES_QTP
  qtpconnection.c
  ikcp.c)

set(LIB_NAME qtp)

add_library(${LIB_NAME} STATIC ${LIB_SOURCES_QTP})

set(QTP_FILETEST qtp_test.c)
set(QTP_PROXY qtproxy_client.c)

add_executable(qtp_filetest ${LIB_SOURCES_QTP} ${QTP_FILETEST})
add_executable(qtproxy_client ${LIB_SOURCES_QTP} ${QTP_PROXY})
