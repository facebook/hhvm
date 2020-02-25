<?hh
// test manually created array
<<__EntryPoint>> function main(): void {
  $x = null;
  $a = darray[];
  $a[0] = 'zero';
  $a[1] = 'one';
  var_dump(current($a)); // should print 'zero'
  // test static array
  $a = varray['zero', 'one'];
  var_dump(current($a));
  // test packed array
  $a = varray['zero', ($x ? 'one' : 'one')];
  var_dump(current($a));
}
