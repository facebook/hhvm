<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

coroutine function some_coroutine(): bool { return false; }

coroutine function return_suspend_in_try_catch(): bool {
  try {
    return suspend some_coroutine();
  } catch (Exception $e) {
    return true;
  }
}
