<?hh


function f($a) {
  var_dump((bool)$a);
}


<<__EntryPoint>>
function main_bool_cast() {
f($GLOBALS['GLOBALS']);
f(darray['a' => 'b']);
f(varray['a']);
f(varray[]);
}
