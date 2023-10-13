//// def.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N as nothing = nothing;

//// use.php
<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {}

function expect(Inv<N> $_): void {}

function test<T as nothing>(Inv<T> $x): void {
  expect($x);
}
