<?hh 

class foo { }

<<__EntryPoint>>
function main_entry(): void {

  $b = new stdClass; $a = clone clone $b;
  var_dump($a == $b);


  $b = new stdClass; $c = clone clone clone $b;
  var_dump($a == $b, $b == $c);

  $b = new foo; $a = $b; $d = clone $a;
  var_dump($a == $d, $b == $d, $c == $a);
}
