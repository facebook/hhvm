<?hh


<<__EntryPoint>>
function main_array_equality() {
$a = function($a, $b) { return $a + $b; };
$aa = darray[
  "parameter" => darray[
    '$a' => '<required>',
    '$b' => '<required>'
  ]
];

var_dump($a == $aa);
}
