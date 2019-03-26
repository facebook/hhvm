<?hh

class Hey { public static $x = "yup\n"; }

function foo(&$cls) {
  $cls = "Hey";
  echo $cls::$x;
}
foo(&$boxer);
