<?hh

class C {
  const foo=1;
}
<<__EntryPoint>>
function main_entry(): void {
  //New instance of class C - defined below
  $rc = new ReflectionClass("C");

  //Check if C has constant foo
  var_dump($rc->hasConstant('foo'));

  //C should not have constant bar
  var_dump($rc->hasConstant('bar'));
}
