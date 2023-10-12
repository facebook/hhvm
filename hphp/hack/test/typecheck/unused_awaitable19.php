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
      // This is a bool
      $is_valid = $this->syncCheck();
    } else {
      // This is an Awaitable<bool> (author forgot to await)
      $is_valid = $this->genAsyncCheck();
    }

    // This is a truthy check on (bool | Awaitable<bool>) which is sketchy
    if ($is_valid) {
      return 2;
    } else {
      return 3;
    }
  }
}
