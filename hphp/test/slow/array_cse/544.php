<?hh

function blocker() :mixed{
 print 'block';
 }
function id($x) :mixed{
 return $x;
 }
function f($x, $y) :mixed{
  $y = $x[$y[0]] ? $x[$y[0]] : id($x[$y[0]]);
  blocker();
  var_dump($y);
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
