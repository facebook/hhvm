<?hh // experimental
$count = 0;
do {
  let x = $count + 1;
  var_dump(x);
  $count++;
} while ($count < 5);
