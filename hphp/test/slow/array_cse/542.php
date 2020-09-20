<?hh

function blocker() {
 print 'block';
 }
function f($x) {
  $x = (string) $x;
  blocker();
  var_dump($x[0]);
  var_dump($x[0]);
}

<<__EntryPoint>>
function main_542() {
var_dump('foo');
}
