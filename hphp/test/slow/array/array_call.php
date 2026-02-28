<?hh

class C {
  static public function foo() :mixed{
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$x = vec['C', var_dump<>];
$x();
}
