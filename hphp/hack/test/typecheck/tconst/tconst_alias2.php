<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
interface I<TGeneric as num> {}

abstract class Foo {

  abstract const type TBar as Bar;

  private function __construct(private this::TBar $bar) {}

  final protected function go(I<this::TBar::Tnum> $x): void {
    $this->bar->go($x);
  }
}
abstract class Bar {
  abstract const type Talias as num;
  const type Tnum = this::Talias;

  final public function go(I<this::Tnum> $x): this {
    return $this;
  }
}
