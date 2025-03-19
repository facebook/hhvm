<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class C {
  public function foo(): vec<mixed> { return vec[null]; }
}

function testit(?C $s):?vec<nonnull> {
  return $s?->foo();
}

<<__EntryPoint>>
function main():nonnull {
  $v = testit(new C());
  return $v is null ? 0 : $v[0];
}
