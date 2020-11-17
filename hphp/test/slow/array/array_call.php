<?hh

class C {
  static public function foo() {
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$x = varray['C', var_dump<>];
$x();
}
