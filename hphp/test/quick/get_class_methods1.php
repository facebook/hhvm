<?hh


trait T {
  private function bar() {}
  public function foo() {}
}

class A {
  use T;
}
<<__EntryPoint>> function main(): void {
print_r(get_class_methods('A'));
}
