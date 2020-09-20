<?hh

class A {
  public function b() {
    $cl = function() {
      return $this->c();
    };
    yield $cl();
  }
  private function c() {
    return 'A';
  }
}
<<__EntryPoint>> function main(): void {
$a = new A;
foreach ($a->b() as $c) {
  print "$c\n";
}
}
