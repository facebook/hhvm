<?hh

class C {
  public static function foo(): Closure {
    return $x ==> $x;
  }
}

<<__EntryPoint>>
function main() {
  var_dump(C::foo()(123));
}
