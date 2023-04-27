<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class C {
  public async function foo():Awaitable<int> {
    return 3;
  }
  public function boo():Awaitable<int> {
    return $this->foo();
  }
}
class D extends C {
  public async function bar():Awaitable<int> {
    return 4;
  }
  // Illegal: do not permit non-async methods to override async ones
  public function foo():Awaitable<int> {
    return $this->bar();
  }
  // Legal: allow async methods to override non-async ones
  public async function boo():Awaitable<int> {
    return 5;
  }
}
