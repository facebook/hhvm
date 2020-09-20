<?hh // partial

class A_f11 {
  <<__Rx, __Mutable>>
    public function f(): void {
  }
}

<<__Rx, __MutableReturn>>
function get(int $x): A_f11 {
  return new A_f11();
}


<<__Rx>>
function f_f11(): void {
  1 |> get($$)->f();
}
