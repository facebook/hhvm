<?hh

class A {}

function a() :mixed{
  $b = new A;
  $c = function() use ($b) {
    return $b;
  };
  return $c();
}
<<__EntryPoint>> function main(): void {
var_dump(a());
}
