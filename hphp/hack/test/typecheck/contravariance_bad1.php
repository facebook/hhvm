<?hh
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

function foo():void { }
function MakeIt(): ISettable<B> {
  // If we write new Box() then Hack
  // incorrectly accepts this: it doesn't check the constraint on T
  $x = new Box<A>();
  // So here $x has type Box<v> with v <: A
  // Now we do Box<v> <: ISettable<B>
  // So ISettable<v> <: ISettable<B>
  // So B <: v by contravariance
  // By transitivity we should therefore check B <: A. Not true!
  return $x;
}

function BreakIt(): void {
  $x = MakeIt();
  $x->Set(new B());
  $x->CallBoo();
}

//BreakIt();
