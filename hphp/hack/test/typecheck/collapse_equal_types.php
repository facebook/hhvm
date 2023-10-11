<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait FooTrait {
  public function do(): int {
    return 0;
  }
}

final class Foo {
  use FooTrait;
}
final class Bar {
  use FooTrait;
}

function sandbox(bool $b): void {
  if ($b) {
    $f = new Foo();
  } else {
    $f = new Bar();
  }
  $x = $f->do(); // type of $x reported as (int | int)
  hh_show($x);
}

