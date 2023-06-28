<?hh


<<__EntryPoint>>
function main_array_equality() :mixed{
$a = function($a, $b) { return $a + $b; };
$aa = darray[
  "parameter" => darray[
    '$a' => '<required>',
    '$b' => '<required>'
  ]
];

var_dump($a == $aa);
}
