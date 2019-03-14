//// def.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

newtype EmptierThanNothing as nothing = nothing;

//// use.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_nothing(nothing $_): void {}

function test(EmptierThanNothing $x): void {
  expect_nothing($x);
}
