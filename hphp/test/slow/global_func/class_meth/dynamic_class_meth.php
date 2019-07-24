<?hh

class C {
  static public function foo() {
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$m = class_meth(C::class, 'foo');
var_dump($m());
}
