<?hh
function cube($s1) :mixed{ return $s1*$s1*$s1; }


<<__EntryPoint>>
function main_array_map() :mixed{
$a = vec[1, 2, 3, 4, 5];
$b = array_map(cube<>, $a);
var_dump($b);
$b = array_map(null, $a);
var_dump($b);
$b = array_map(null, dict['x' => 6, 0 => 7]);
var_dump($b);
var_dump(
  array_map(
    null,
    dict['x' => 6, 0 => 7],
    vec[vec['a', 'b'], true]
  )
);
}
