<?hh

function test($a, $b, $i) :mixed{
  list($a[$i++], $a[$i++], $a[$i++]) = $b;
  var_dump($a);
  }

<<__EntryPoint>>
function main_46() :mixed{
test(darray[], varray['x', 'y', 'z'], 0);
}
