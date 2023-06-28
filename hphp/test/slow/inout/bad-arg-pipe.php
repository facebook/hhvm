<?hh

class C {
  public static function f(inout mixed $x) :mixed{
    var_dump($x);
  }
  public function g() :mixed{
    1 |> C::f(inout $$);
  }
}

<<__EntryPoint>>
function main():mixed{
  (new C)->g();
}

