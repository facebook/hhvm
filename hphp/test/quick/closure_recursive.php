<?hh
class A {
  public function b() :mixed{
    return function() {
      return function() {
        return $this->c();
      };
    };
  }
  private function c() :mixed{
    return 91;
  }
}
<<__EntryPoint>> function main(): void {
$a = new A;
$b = $a->b();
$first = $b();
$second = $first();
var_dump($second);
}
