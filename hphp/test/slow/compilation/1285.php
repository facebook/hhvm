<?hh

function foo($p1="/.*/", $p2="//") :mixed{
  var_dump($p1, $p2);
}

<<__EntryPoint>>
function main_1285() :mixed{
foo();
}
