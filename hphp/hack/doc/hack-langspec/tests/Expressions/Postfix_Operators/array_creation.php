<?hh // strict

namespace NS_array_creation;

function main(): void {
  $a = [1 => 10, 'xx' => 20, 4.5 => 30, false => 40, '-10' => 50];
  var_dump($a);
}

/* HH_FIXME[1002] call to main in strict*/
main();

