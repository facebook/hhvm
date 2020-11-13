<?hh

<<__NEVER_INLINE>>
function dup($x, $y) {
  return $x;
}

abstract class MyClass {
  abstract const type TTest;

  <<__NEVER_INLINE>>
  function doCheck($v) {
    $type = type_structure(static::class, 'TTest');
    $x = dup($v, $type['classname']);
    var_dump($x);
  }
}

class MyClassImpl extends MyClass {
  const type TTest = MyClass;
}

<<__EntryPoint>>
function main() {
  $v = new MyClassImpl();
  new MyClassImpl()->doCheck($v);
  print "done\n";
}
