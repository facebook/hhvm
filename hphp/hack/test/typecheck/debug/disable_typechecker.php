<?hh

<<__DisableTypecheckerInternal>>
function f(): void {
  3 + "4";
}

class C {
  <<__DisableTypecheckerInternal>>
  public function g(): void {
    3 + "4";
  }
}
