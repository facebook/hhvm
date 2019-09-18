<?hh

class C {
  public function foo() {
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$m = inst_meth(new C, 'foo');
var_dump($m());
}
