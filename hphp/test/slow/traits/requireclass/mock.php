<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;
}

final class C {
  use T;
}

<<__MockClass>>
final class MockC extends C {}

<<__EntryPoint>>
function main() : void {
  new Mockc();
  echo "done.\n";
}
