<?hh

class A {
  public function b() {
    return 'c';
  }
}
<<__EntryPoint>> function main(): void {
$d = new A;
var_dump((clone $d)->b());
}
