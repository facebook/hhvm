<?hh

class A {
  public function b() :AsyncGenerator<mixed,mixed,void>{
    $cl = function() {
      yield $this->c();
    };
    yield $cl();
  }
  private function c() :mixed{
    return 'A';
  }
}
<<__EntryPoint>> function main(): void {
$a = new A;
foreach ($a->b() as $c) {
  foreach ($c as $d) {
    print "$d\n";
  }
}
}
