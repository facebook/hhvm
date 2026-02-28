<?hh

// references a static variable of an XHP class in various contexts

class :fb:thing {
  public static int $x = 42;
}

<<__EntryPoint>>
function main_class_name() :mixed{
  var_dump(:fb:thing::$x);
  :fb:thing::$x = :fb:thing::$x + 1;
  ++:fb:thing::$x;
  :fb:thing::$x++;
  var_dump(:fb:thing::$x);

  $classname = :fb:thing::class;
  var_dump($classname::$x);
  $classname::$x = $classname::$x + 1;
  ++$classname::$x;
  $classname::$x++;
  var_dump($classname::$x);
}
