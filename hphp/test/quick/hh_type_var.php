<?hh

abstract class A<T> {
  abstract public function f1(T $x):mixed;
  abstract public function f2(?T $x):mixed;
  abstract public function f3(<<__Soft>> T $x):mixed;
  abstract public function f4(<<__Soft>> ?T $x):mixed;
  abstract public function f5(T $x):mixed;
  abstract public function f6(?T $x):mixed;
  abstract public function f7(<<__Soft>> T $x):mixed;
  abstract public function f8(<<__Soft>> ?T $x):mixed;
}

abstract class B<T> extends A<T> {}

class C extends B<int> {
  public function f1(int $x) :mixed{ }
  public function f2(?int $x) :mixed{ }
  public function f3(<<__Soft>> int $x) :mixed{ }
  public function f4(<<__Soft>> ?int $x) :mixed{ }
  public function f5(<<__Soft>> int $x) :mixed{ }
  public function f6(<<__Soft>> ?int $x) :mixed{ }
  public function f7(int $x) :mixed{ }
  public function f8(?int $x) :mixed{ }
}

interface D<T> {
  public function f1(T $p5):mixed;
  public function f2(?T $p6):mixed;
  public function f3(<<__Soft>> T $p5):mixed;
  public function f4(<<__Soft>> ?T $p6):mixed;
  public function f5(T $p5):mixed;
  public function f6(?T $p6):mixed;
  public function f7(<<__Soft>> T $p5):mixed;
  public function f8(<<__Soft>> ?T $p6):mixed;
}

class E implements D<int> {
  public function f1(int $p7) :mixed{ }
  public function f2(?int $p8) :mixed{ }
  public function f3(<<__Soft>> int $p7) :mixed{ }
  public function f4(<<__Soft>> ?int $p8) :mixed{ }
  public function f5(<<__Soft>> int $p7) :mixed{ }
  public function f6(<<__Soft>> ?int $p8) :mixed{ }
  public function f7(int $p7) :mixed{ }
  public function f8(?int $p8) :mixed{ }
}

abstract class F<T1, T2> {
  abstract public function f1(T1 $x, T2 $y):mixed;
}

class G<T2, T1> extends F<T1, T2> {
  public function f1(T2 $x, T1 $y) :mixed{ }
}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
