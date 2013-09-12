<?hh

abstract class A<T> {
  abstract public function f1(T $x);
  abstract public function f2(?T $x);
  abstract public function f3(@T $x);
  abstract public function f4(@?T $x);
  abstract public function f5(T $x);
  abstract public function f6(?T $x);
  abstract public function f7(@T $x);
  abstract public function f8(@?T $x);
}

abstract class B<T> extends A<T> {}

class C extends B<int> {
  public function f1(int $x) { }
  public function f2(?int $x) { }
  public function f3(@int $x) { }
  public function f4(@?int $x) { }
  public function f5(@int $x) { }
  public function f6(@?int $x) { }
  public function f7(int $x) { }
  public function f8(?int $x) { }
}

interface D<T> {
  public function f1(T $p5);
  public function f2(?T $p6);
  public function f3(@T $p5);
  public function f4(@?T $p6);
  public function f5(T $p5);
  public function f6(?T $p6);
  public function f7(@T $p5);
  public function f8(@?T $p6);
}

class E implements D<int> {
  public function f1(int $p7) { }
  public function f2(?int $p8) { }
  public function f3(@int $p7) { }
  public function f4(@?int $p8) { }
  public function f5(@int $p7) { }
  public function f6(@?int $p8) { }
  public function f7(int $p7) { }
  public function f8(?int $p8) { }
}

abstract class F<T1, T2> {
  abstract public function f1(T1 $x, T2 $y);
}

class G<T2, T1> extends F<T1, T2> {
  public function f1(T2 $x, T1 $y) { }
}
echo "Done\n";
