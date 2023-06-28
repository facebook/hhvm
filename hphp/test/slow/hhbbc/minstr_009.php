<?hh

function asd() :mixed{ return 'xxxxx'; }
function bar() :mixed{ return mt_rand() ? 'a' : 'b'; }
function foo() :mixed{
  $x = asd();
  $x[0] = bar();
  $x[0] = 'a';
  echo $x;
  echo "\n";
  var_dump($x);
}

<<__EntryPoint>>
function main_minstr_009() :mixed{
foo();
}
