<?hh 

class foo { }

<<__EntryPoint>>
function main_entry(): void {

  $a = clone clone $b = new stdClass;
  var_dump($a == $b);


  $c = clone clone clone $b = new stdClass;
  var_dump($a == $b, $b == $c);

  $d = clone $a = $b = new foo;
  var_dump($a == $d, $b == $d, $c == $a);
}
