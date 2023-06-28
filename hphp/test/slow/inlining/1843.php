<?hh

function pid($x) :mixed{
  var_dump($x);
  return $x;
}
function f($x) :mixed{
  return $x;
}
function ttest() :mixed{
  return f(pid('arg1'),pid('arg2'));
}

<<__EntryPoint>>
function main_1843() :mixed{
ttest();
}
