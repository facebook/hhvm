<?hh

class C {
  public static function f(inout mixed $x) {
    var_dump($x);
  }
  public function g() {
    1 |> C::f(inout $$);
  }
}

<<__EntryPoint>>
function main(){
  (new C)->g();
}

