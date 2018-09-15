<?hh // experimental

let max = 10;
$i = 0;
do {
  let twice = 2 * $i;
  $i++;
  var_dump(twice);
} while (twice < max);
