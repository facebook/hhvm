<?hh

abstract class C {
}

class D extends C {
}

interface I {
  public function bar(): ?D;
}

trait T {
  private function asI(): I {
    return $this as I;
  }

  final public function foo(): ?C {
    $ret = $this->asI()->bar();
    $ret = $ret ?as C;
    return $ret;
  }
}

abstract class E implements I {
  use T;
};

class F implements I {
  use T;
  public function bar(): ?D { return null; }
}

class G implements I {
  public function bar(): D { return new D(); }
}

class H {
  public function bar() :mixed{ return vec[]; }
}

abstract class X1 extends E {
}
abstract class X2 extends E {
}

<<__EntryPoint>>
function main() :mixed{
  $f = new F();
  $f->foo();
  echo "done\n";
}
