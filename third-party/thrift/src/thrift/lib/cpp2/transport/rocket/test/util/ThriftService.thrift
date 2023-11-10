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

namespace cpp2 testutil.testservice

include "thrift/annotation/cpp.thrift"

struct Message {
  1: string message;
  2: i64 timestamp;
}

exception Error {}

exception FirstEx {
  1: i64 errCode;
  2: string errMsg;
}

exception SecondEx {
  1: i64 errCode;
}

service StreamService {
  // Echo input value
  i32 echo(1: i32 value);

  // Generate numbers between `from` to `to`.
  stream<i32> range(1: i32 from, 2: i32 to);
  // ... with a sleep in between iterations
  stream<i32> slowRange(1: i32 from, 2: i32 to, 3: i32 millis);
  // Generate strings of size 1KB
  stream<string> buffers(1: i32 count);
  // Generate strings of specified size
  stream<string> customBuffers(1: i32 count, 2: i32 size);

  stream<i32> slowCancellation();

  // These method will not be overiden, so the default implementation will be
  // used. If client calls these methods, it should not cause any crash and it
  // should end gracefully
  stream<Message> nonImplementedStream(1: string sender);

  stream<Message> returnNullptr();

  i32, stream<Message> throwError() throws (1: Error ex);

  i32, stream<i32> leakCheck(1: i32 from, 2: i32 to);
  i32, stream<i32> leakCheckWithSleep(1: i32 from, 2: i32 to, 3: i32 sleepMs);
  i32 instanceCount();

  i32, stream<i32> sleepWithResponse(1: i32 timeMs);
  stream<i32> sleepWithoutResponse(1: i32 timeMs);

  i32, stream<i32> streamServerSlow();

  // Simple chat scenario
  void sendMessage(1: i32 messageId, 2: bool complete, 3: bool error);
  stream<i32> registerToMessages();

  stream<Message throws (1: FirstEx e)> streamThrows(1: i32 whichEx) throws (
    1: SecondEx e,
  );

  i32, stream<Message throws (1: FirstEx e)> responseAndStreamThrows(
    1: i32 whichEx,
  ) throws (1: SecondEx e);

  stream<i32> requestWithBlob(1: binary_9305 val);

  @cpp.ProcessInEbThreadUnsafe
  stream<i32> leakCallback();

  @cpp.ProcessInEbThreadUnsafe
  i32, stream<i32> orderRequestStream();
  @cpp.ProcessInEbThreadUnsafe
  i32 orderRequestResponse();

  stream<i32> leakPublisherCheck();
}

# OldVersion and NewVersion services will be used to test the behavior
# when the service functions change
service OldVersion {
  // Unchanged methods
  i32 AddOne(1: i32 number);
  stream<i32> Range(1: i32 from, 2: i32 length);
  i32, stream<i32> RangeAndAddOne(1: i32 from, 2: i32 length, 3: i32 number);

  // This method is deleted in the NewVersion
  void DeletedMethod();
  stream<Message> DeletedStreamMethod();
  Message, stream<Message> DeletedResponseAndStreamMethod();

  // This streaming method is going to be changed to a Request&Response method
  stream<Message> StreamToRequestResponse();

  Message, stream<Message> ResponseandStreamToRequestResponse();

  // This Request&Response method is going to be changed to a streaming method
  Message RequestResponseToStream();

  Message RequestResponseToResponseandStream();
}

service NewVersion {
  i32 AddOne(1: i32 number);
  stream<i32> Range(1: i32 from, 2: i32 length);
  i32, stream<i32> RangeAndAddOne(1: i32 from, 2: i32 length, 3: i32 number);
  void StreamToRequestResponse();
  void ResponseandStreamToRequestResponse();
  stream<Message> RequestResponseToStream();
  Message, stream<Message> RequestResponseToResponseandStream();
}

service TransportUpgrade {
  i32 addTwoNumbers(1: i32 num1, 2: i32 num2);
  i32 add(1: i32 x);
  oneway void noResponse(1: string param);
}

// The following were automatically generated and may benefit from renaming.
@cpp.Type{name = "folly::IOBuf"}
typedef binary binary_9305
