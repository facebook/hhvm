//// def1.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N1 as nothing = nothing;

//// def2.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype N2 as nothing = nothing;

//// use.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<T> {}

function expect(Inv<N1> $_): void {}

function test(Inv<N2> $x): void {
  expect($x);
}
