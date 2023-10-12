<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function Foo(int $i): void;
}
interface J {
  public function Bar(string $s): void;
}

class C<T as J as I> {
  public function __construct(private T $item) {}
  public function CallFoo(I $x): void {
    $x->Foo(3);
  }
  public function CallBar(J $x): void {
    $x->Bar('a');
  }
  public function Boo(T $x): void {
    // This verifies that subtyping T <: I and T <: J is working
    $this->CallFoo($x);
    $this->CallBar($x);
    $x->Foo(3);
    $x->Bong('a');
  }
}
