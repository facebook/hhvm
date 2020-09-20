<?hh

class A {
  private ?dict<string, mixed> $d;

  private dict<string, mixed> $d2 = dict[];

  <<__Rx, __Mutable>>
  private function my_test_function(): void {
   $this->d as nonnull;
   $this->d['foo'] = 'bar';
   $this->d2['foo'] = 'bar';
  }
}
