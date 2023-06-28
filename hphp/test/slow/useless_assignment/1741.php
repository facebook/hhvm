<?hh

function out($a) :mixed{
  echo $a,'\n';
}
function test($a) :mixed{
  $a ? out('?a') : out(':a');
  $a ? out('+a') : 0;
  $a ? 0 : out('-a');
  $a && out('&&a');
  $a || out('||a');
  $b = $c = 0;
  $a || (($b = 5) + ($c = 6));
  out($b);
 out($c);
}

<<__EntryPoint>>
function main_1741() :mixed{
test(0);
test('foo');
}
