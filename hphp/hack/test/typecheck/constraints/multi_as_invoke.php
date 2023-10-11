<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {
  public function Foo(int $i): void;
}
interface J {
  public function Bar(string $s): arraykey;
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
    // This verifies that invocation through J or I is working
    $x->Foo(3);
    $x->Bar('a');
  }
}

class D implements I {
  public function Foo(int $i): void {
    echo 'D::Foo with ', $i;
  }
}
class E implements J {
  public function Bar(string $s): arraykey {
    echo 'E::Bar with ', $s;
    return 3;
  }
}
class F implements I, J {
  public function Foo(int $i): void {
    echo 'F::Foo with ', $i;
  }
  public function Bar(string $s): arraykey {
    echo 'F::Bar with ', $s;
    return 'a';
  }
}
class Test {
  public function Doit(): void {
    $x = new C(new F());
    $x->Boo(new F());
  }
}
