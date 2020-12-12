<?hh

class C {
  static function f<reify T>($x) {
    echo "in C::f\n";
    var_dump($x is T);
  }
}

trait T {
  static function f<reify T>($x) {
    echo "in D::f\n";
    var_dump($x is T);
    parent::f<T>($x);
  }
}

class D extends C {
  use T;
}

<<__EntryPoint>>
function main() {
  D::f<int>(1);
  D::f<int>("hi");
}
