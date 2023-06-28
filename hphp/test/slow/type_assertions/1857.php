<?hh

function f($x) :mixed{
  while (is_array($x) && isset($x[0])) $x = $x[0];
  var_dump($x);
}
function g($x) :mixed{
  for (;
 is_array($x) && isset($x[0]);
 $x = $x[0]);
  var_dump($x);
}
function h($x) :mixed{
  if (!is_array($x) || !isset($x[0])) return;
  do {
    $x = $x[0];
  }
 while (is_array($x) && isset($x[0]));
  var_dump($x);
}

<<__EntryPoint>>
function main_1857() :mixed{
f(varray[varray[varray[varray['hello']]]]);
g(varray[varray[varray[varray['hello']]]]);
h(varray[varray[varray[varray['hello']]]]);
}
