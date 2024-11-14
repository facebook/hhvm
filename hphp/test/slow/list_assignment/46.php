<?hh

function test($a, $b, $i) :mixed{
  list($a[$i], $a[$i+1], $a[$i+2]) = $b;
  var_dump($a);
  }

<<__EntryPoint>>
function main_46() :mixed{
test(dict[], vec['x', 'y', 'z'], 0);
}
