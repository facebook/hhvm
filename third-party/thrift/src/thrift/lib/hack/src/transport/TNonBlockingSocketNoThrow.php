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
 * Wrapper class over TNonBlockingSocket that suppresses
 * the exception "operation in progress 115" when the open()
 * is called. This is helpful for classes that don't
 * handle properly this event. Ie. TFramedTransport.
 */
<<Oncalls('thrift')>> // @oss-disable
final class TNonBlockingSocketNoThrow extends TNonBlockingSocket {
  <<__Override>>
  public function open()[leak_safe]: void {
    try {
      parent::open();
    } catch (ExceptionWithPureGetMessage $e) {
      $op_in_progress = (
        PHP\strpos($e->getMessage(), "socket_connect error") !== false
      ) &&
        (PHP\strpos($e->getMessage(), "[115]") !== false);
      if (!$op_in_progress) {
        throw $e;
      }
    }
  }
}
