<?hh

class foo {
  public function bar() {
    $z = function() {};
    $zz = function() {};
    $zzz = function() {};
    $zzzz = function() {};
    $zzzzz = function() {};
    $zzzzzz = function() {};
  }
}
<<__EntryPoint>> function main(): void {
  $l = new foo;
  $l->bar();
}
