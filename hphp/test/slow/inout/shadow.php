<?hh

function wrap($x) :mixed{
  return ', '.$x.'!';
}

function io(inout $x, $y) :mixed{
  $x = 'hello';
  return $y;
}

function main() :mixed{
  $f = varray[1, 'world', 3];
  $ret = io(inout $f, wrap($f[1]));
  echo $f.$ret."\n";

  $f = varray[null, 'x', varray[1, 'world', 3]];
  $ret = io(inout $f, wrap($f[2][1]));
  echo $f.$ret."\n";

  $f = varray[null, 'x', varray[1, 'orl', 3]];
  $ret = io(inout $f, wrap('w'.$f[2][1].'d'));
  echo $f.$ret."\n";

  $f = 'orl';
  $ret = io(inout $f, wrap('w'.$f.'d'));
  echo $f.$ret."\n";

  $f = 'world';
  $ret = io(inout $f, wrap($f ^ 'world' ^ $f));
  echo $f.$ret."\n";

  $f = 'world';
  $ret = io(inout $f, wrap($f));
  echo $f.$ret."\n";
}


<<__EntryPoint>>
function main_shadow() :mixed{
main();
}
