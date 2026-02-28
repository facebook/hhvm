<?hh

class C {
  public static function f(inout mixed $x) :mixed{
    var_dump($x);
  }
  public function g() :mixed{
    C::f(inout $this);
  }
}

<<__EntryPoint>>
function main():mixed{
  (new C)->g();
}

