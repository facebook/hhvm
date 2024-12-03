<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
