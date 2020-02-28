<?hh

function g(inout $x) {
 var_dump($x);
}
function f($x) {
  if (is_array($x)) {

    var_dump($x);
    try {
      var_dump($x[0]);
      if (is_array($x[0])) var_dump($x[0][1]);
    } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
  if (is_array($x) && $x) {
    g(inout $x);
  }
}

<<__EntryPoint>>
function main_1853() {
f(varray[]);
f(varray[0, 1]);
f(varray[darray[1 => 1]]);
}
