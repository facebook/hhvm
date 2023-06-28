<?hh

function blocker() :mixed{
 print 'block';
 }
function f($x) :mixed{
  $x = (string) $x;
  blocker();
  var_dump($x[0]);
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_542() :mixed{
var_dump('foo');
}
