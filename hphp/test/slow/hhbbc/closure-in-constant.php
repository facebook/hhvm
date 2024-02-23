<?hh

enum class B : int {
  int B1 = ($x ==> $x)(__hhvm_intrinsics\launder_value(123));
}

<<__EntryPoint>>
function main(): void {
  var_dump(B::B1);
}
