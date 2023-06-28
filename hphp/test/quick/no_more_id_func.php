<?hh

class A {
  public function b() :mixed{
    return 'id';
  }
}
<<__EntryPoint>> function main(): void {
var_dump((new A)->b());
}
