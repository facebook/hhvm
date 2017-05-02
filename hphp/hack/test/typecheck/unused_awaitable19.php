<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function syncCheck(): bool {
    return true;
  }
  public async function genAsyncCheck(): Awaitable<bool> {
    return true;
  }
  public async function test(bool $fast_path): Awaitable<int> {
    if ($fast_path) {
      $is_valid = $this->syncCheck();
    } else {
      $is_valid = $this->genAsyncCheck(); // BUG HERE!!!
    }

    if ($is_valid) { // No warning here
      return 2;
    } else {
      return 3;
    }
  }
}
