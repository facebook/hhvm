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

interface IResultThriftStruct extends \IThriftStruct {
  abstract const type TResult;
  public function checkForException(): ?\TException;

  /**
   * Attempt to set a exception on this struct.
   * Expected behavior
   * 1. If the exception matches one of this result-struct's
   *    exception types, store a reference to `$e` and return true.
   * 2. Return false otherwise
   */
  public function setException(\Exception $e): bool;
}

abstract class ThriftSyncStructWithResult
  implements IResultThriftStruct, IThriftSyncStruct {
  public ?this::TResult $success;
  public function checkForException(): ?\TException {
    return null;
  }
  public function setException(\Exception $e): bool {
    return false;
  }
}

abstract class ThriftAsyncStructWithResult
  implements IResultThriftStruct, IThriftAsyncStruct {
  public ?this::TResult $success;
  public function checkForException(): ?\TException {
    return null;
  }
  public function setException(\Exception $e): bool {
    return false;
  }
}

abstract class ThriftSyncStructWithoutResult
  implements IResultThriftStruct, IThriftSyncStruct {
  const type TResult = null;

  public function checkForException(): ?\TException {
    return null;
  }
  public function setException(\Exception $e): bool {
    return false;
  }
}

abstract class ThriftAsyncStructWithoutResult
  implements IResultThriftStruct, IThriftAsyncStruct {
  const type TResult = null;

  public function checkForException(): ?\TException {
    return null;
  }
  public function setException(\Exception $e): bool {
    return false;
  }
}
