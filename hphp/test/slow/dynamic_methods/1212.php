<?hh

function t($x) {
  var_dump($x);
}
class z {
  function q() {
    $x = varray[1,2,3];
    array_map(varray['z', 'p'], $x);
  }
  static function p($x) {
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_1212() {
$x = varray[1,2,3];
array_map(fun('t'), $x);
$m = new z();
$m->q();
}
