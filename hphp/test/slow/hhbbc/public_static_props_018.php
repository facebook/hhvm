<?hh

class Something {}

class Asd {
  public static $SOMETHING = 'Something';
}

function foo($y) :mixed{
  if (is_a($y, Asd::$SOMETHING)) {
    echo "was an instance\n";
  } else {
    echo "nope\n";
  }
}


<<__EntryPoint>>
function main_public_static_props_018() :mixed{
foo(new Something);
foo(2);
}
