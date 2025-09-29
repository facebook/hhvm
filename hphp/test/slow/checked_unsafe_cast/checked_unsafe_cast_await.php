<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rights Reserved.

async function make_int():Awaitable<mixed> {
  return 42;
}

enum E : int as int {
  A = 0;
  B = 0;
}
function expect_int(int $_):void { }

class C {
  public async function make_int():Awaitable<int> { return 3; }
  public async function make_nint():Awaitable<?int> { return null; }
}
<<__EntryPoint>>
async function main():Awaitable<void> {
  $s = HH\FIXME\UNSAFE_CAST<mixed,string>(await make_int());
  $v = Vector {5, 6};
  $c = new C();
  $v[E::A] = HH\FIXME\UNSAFE_CAST<mixed, int>(await $c->make_int());
  $v[E::B] = HH\FIXME\UNSAFE_NONNULL_CAST(await $c->make_nint());
}
