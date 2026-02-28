<?hh


function f($a) :mixed{
  var_dump((bool)$a);
}


<<__EntryPoint>>
function main_bool_cast() :mixed{
f(dict['a' => 'b']);
f(vec['a']);
f(vec[]);
}
