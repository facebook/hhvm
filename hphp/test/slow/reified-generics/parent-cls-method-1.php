<?hh

class C {
  static function f<reify T>($x) :mixed{
    echo "in C::f\n";
    var_dump($x is T);
  }
}

class D extends C {
  static function f<reify T>($x) :mixed{
    echo "in D::f\n";
    var_dump($x is T);
    parent::f<T>($x);
  }
}

<<__EntryPoint>>
function main() :mixed{
  D::f<int>(1);
  D::f<int>("hi");
}
