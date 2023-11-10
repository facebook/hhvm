/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace py thrift.util.test_service

include "thrift/annotation/thrift.thrift"

exception UserException1 {
  1: string message;
}

exception UserException2 {
  1: string message;
}

service TestService {
  string getDataById(1: i64 id);
  string getMessage();
  bool hasDataById(1: i64 id);
  void putDataById(1: i64 id, 2: string data);
  void delDataById(1: i64 id);
  void throwUserException() throws (
    1: UserException1 ex1,
    2: UserException2 ex2,
  );
  i64 throwUncaughtException(1: string msg);
}

service PriorityService {
  @thrift.Priority{level = thrift.RpcPriority.BEST_EFFORT}
  bool bestEffort();
  @thrift.Priority{level = thrift.RpcPriority.NORMAL}
  bool normal();
  @thrift.Priority{level = thrift.RpcPriority.IMPORTANT}
  bool important();
  bool unspecified();
} (priority = 'HIGH')

service SubPriorityService extends PriorityService {
  bool child_unspecified();
  @thrift.Priority{level = thrift.RpcPriority.HIGH_IMPORTANT}
  bool child_highImportant();
}
