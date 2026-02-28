<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type Foo = shape(
  ?'asd' => ?int,
);

function bar(Foo $f): void {
  $asd = Shapes::idx($f, 'asd');
  $baz = new Baz($asd);
  $baz->b();
}

class Baz<T> {
  public function __construct(private T $t) {}
  public function b<Tu as nonnull>(): Tu where T = ?Tu {
    return $this->t as nonnull;
  }
}
