<?hh


function f($a) :mixed{
  var_dump((bool)$a);
}


<<__EntryPoint>>
function main_bool_cast() :mixed{
f(darray['a' => 'b']);
f(varray['a']);
f(varray[]);
}
