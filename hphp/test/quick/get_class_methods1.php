<?hh


trait T {
  private function bar() :mixed{}
  public function foo() :mixed{}
}

class A {
  use T;
}
<<__EntryPoint>> function main(): void {
print_r(get_class_methods('A'));
}
