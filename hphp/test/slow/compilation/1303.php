<?hh

function checker($x) :mixed{
  $msg = foo();
  $notice = $msg['title'].'. '.$msg['body'];
  foo();
  list($a,$b) = $x;
  $x = $x['a'];
  $x = $x['b'];
  return $a - $b + $x;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
