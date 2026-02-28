<?hh

class K { const A = 0; const B = 1; }
class Q { const A = 'f'; const B = 'Q::B'; }

<<__EntryPoint>>
function main() :mixed{
  $x = dict[0 => 42, 1 => 1337];
  $y = dict['f' => 10];
  $z = dict['Q::B' => 20];
  $t = dict['f' => 10, 'Q::B' => 20];

  var_dump($x is shape(K::A => int, K::B => int));
  var_dump($y is shape(Q::A => int));
  var_dump($z is shape(Q::B => int));
  var_dump($t is shape(Q::A => int, Q::B => int));

  var_dump($x is shape(Q::A => int, Q::B => int));
  var_dump($y is shape(K::A => int));
  var_dump($z is shape(K::B => int));
  var_dump($t is shape(K::A => int, K::B => int));
}
