<?hh
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
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

/**
 *  Determine (as best as possible) whether the transport can preform
 *  non-blocking read and write operations.
 */
<<Oncalls('thrift')>> // @oss-disable
interface TTransportStatus {

  /**
   *  Test whether the transport is ready for a non-blocking read. It is
   *  possible, though, that a transport is ready for a partial read, but a full
   *  read will block.
   *
   *  In the case a transport becomes unavailable for reading due to an error
   *  an exception should be raised. Any timeout logic should also raise an
   *  exception.
   *
   *  @return bool True if a non-blocking read can be preformed on the
   *               transport.
   */
  public function isReadable()[leak_safe]: bool;

  /**
   *  Test whether the transport is ready for a non-blocking write.
   *
   *  In the case a transport becomes unavailable for writing due to an error
   *  an exception should be raised. Any timeout logic should also raise an
   *  exception.
   *
   *  @return bool True if a non-blocking write can be preformed on the
   *               transport.
   */
  public function isWritable()[leak_safe]: bool;
}
