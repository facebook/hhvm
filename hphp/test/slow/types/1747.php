<?hh

function p(?AnyArray $i = null) :mixed{
  var_dump($i);
  $i = varray[];
}
function q() :mixed{
  p(null);
}

<<__EntryPoint>>
function main_1747() :mixed{
p();
}
