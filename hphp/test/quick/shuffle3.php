<?hh
function f() { $a = array(); $a[] = 1; return $a; }
function nonHphpArray( ) {
  apc_add('foo', array(1, 2, 3));
  return apc_fetch('foo');
}
function test1() {
  return f()[0];
}
<<__EntryPoint>> function main(): void {
  var_dump(test1());
}
