<?hh
// (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

type SomeInfo = shape(
  ?'extra' => ?string,
);

function foo(mixed $s): void {}

function test_is_nonnull(SomeInfo $info): void {
  if (Shapes::idx($info, 'extra') is nonnull) {
    foo($info['extra']); // No error
  }
}

function test_isnt_null(SomeInfo $info): void {
  if (!(Shapes::idx($info, 'extra') is null)) {
    foo($info['extra']); // No error
  }
}
