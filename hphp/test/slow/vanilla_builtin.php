<?hh

<<__NEVER_INLINE>>
function dup($x, $y) :mixed{
  return $x;
}

abstract class MyClass {
  abstract const type TTest;

  <<__NEVER_INLINE>>
  function doCheck($v) :mixed{
    $type = type_structure(static::class, 'TTest');
    $x = dup($v, $type['classname']);
    var_dump($x);
  }
}

class MyClassImpl extends MyClass {
  const type TTest = MyClass;
}

<<__EntryPoint>>
function main() :mixed{
  $v = new MyClassImpl();
  new MyClassImpl()->doCheck($v);
  print "done\n";
}
