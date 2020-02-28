<?hh

function test($a, $b, $i) {
  list($a[$i++], $a[$i++], $a[$i++]) = $b;
  var_dump($a);
  }

<<__EntryPoint>>
function main_46() {
test(varray[], varray['x', 'y', 'z'], 0);
}
