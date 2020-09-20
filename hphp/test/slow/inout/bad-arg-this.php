<?hh

class C {
  public static function f(inout mixed $x) {
    var_dump($x);
  }
  public function g() {
    C::f(inout $this);
  }
}

<<__EntryPoint>>
function main(){
  (new C)->g();
}

