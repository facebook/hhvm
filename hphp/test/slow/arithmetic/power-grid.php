<?hh

function inline_dump($val) {
  ob_start();
  var_dump($val);
  $contents = ob_get_contents();
  ob_end_clean();
  return trim(preg_replace('/[\r\n\s]+/', ' ', $contents));
}

<<__EntryPoint>>
function main_power_grid() {
error_reporting(E_ALL | E_NOTICE | E_STRICT);

$args = varray[
  null,
  true,
  false,
  2,
  3.0,
  '',
  '4',
  varray[],
  varray[5],
  fopen('php://memory', 'w+'),
  $closeme = fopen('php://memory', 'w+'),
];
fclose($closeme);

foreach($args as $x) {
  foreach($args as $y) {
    echo inline_dump($x), ' ** ',
         inline_dump($y), ' => ',
         inline_dump($x ** $y), "\n";
  }
}
}
