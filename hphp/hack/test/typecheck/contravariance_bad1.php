<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface ISettable<-T> {
  public function Set(?T $x): void;
  public function CallBoo(): void;
}

class A {
  public function Boo(): void {
    echo 'Boo';
  }
}
class B {}

final class Box<T as A> implements ISettable<T> {
  public function __construct() {}
  private ?T $item;
  public function Set(?T $x): void {
    $this->item = $x;
  }
  public function CallBoo(): void {
    if ($this->item !== null) {
      $this->item->Boo();
    }
  }
}

function MakeIt(): ISettable<B> {
  $x = new Box();
  return $x;
}

function BreakIt(): void {
  $x = MakeIt();
  $x->Set(new B());
  $x->CallBoo();
}

//BreakIt();
