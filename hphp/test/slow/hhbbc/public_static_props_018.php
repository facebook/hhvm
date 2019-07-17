<?hh

class Something {}

class Asd {
  static $SOMETHING = 'Something';
}

function foo($y) {
  if (is_a($y, Asd::$SOMETHING)) {
    echo "was an instance\n";
  } else {
    echo "nope\n";
  }
}


<<__EntryPoint>>
function main_public_static_props_018() {
foo(new Something);
foo(2);
}
