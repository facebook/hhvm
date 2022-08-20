# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

PROJECT(ThriftTransport_CPP2_Example CXX)

add_executable(
  example_server

  server/EchoService.h
  server/EchoService.cpp
  server/ChatRoomService.h
  server/ChatRoomService.cpp
  server/ExampleServer.cpp
)
target_link_libraries(
  example_server
  thriftcpp2
  chatroom-cpp2
  ${LIBGFLAGS_LIBRARY}
  ${GLOG_LIBRARIES}
)
install(
  TARGETS example_server
  RUNTIME DESTINATION bin/cpp2/
)

add_executable(
  chatroom_client

  client/ChatRoomClient.cpp
)
target_link_libraries(
  chatroom_client

  chatroom-cpp2
)
install(
  TARGETS chatroom_client
  RUNTIME DESTINATION bin/cpp2/
)

add_executable(
  echo_client

  client/EchoClient.cpp
)
target_link_libraries(
  echo_client

  chatroom-cpp2
)
install(
  TARGETS echo_client
  RUNTIME DESTINATION bin/cpp2/
)
