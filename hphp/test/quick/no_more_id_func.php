<?hh

class A {
  public function b() {
    return 'id';
  }
}
<<__EntryPoint>> function main(): void {
var_dump((new A)->b());
}
