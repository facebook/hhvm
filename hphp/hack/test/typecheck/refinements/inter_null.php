<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(?int $x): void {
  if ($x is null) {
    expect<null>($x);
  } else {
    // should show `int`, not unsimplified forms like `(int | nothing)`
    hh_show($x);
    expect<int>($x);
  }
  if ($x is nonnull) {
    expect<int>($x);
  } else {
    expect<null>($x);
  }
}

function expect<T>(T $x) : void {}
