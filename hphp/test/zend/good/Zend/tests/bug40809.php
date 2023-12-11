<?hh <<__EntryPoint>> function main(): void {
error_reporting(E_ALL|E_STRICT);

$num_increments = 100;
$num_repeats = 1000;
$increment = 50;

/* Create some more holes to give the memory allocator something to
 * work with. */
$num = 5000;
$a = dict[];
for ($i=0; $i<$num; $i++) {
  $a[$i] = vec[1];
}
$b = dict[];
for ($i=0; $i<$num; $i++) {
  $b[$i] = $a[$i][0];
}
unset($a);

for ($i=0;$i<$num_repeats;$i++) {
  $evil = "";
  for ($j=0;$j<$num_increments;$j++) {
    $evil .= str_repeat("a", $increment);
  }
  unset($evil);
}
echo "ok\n";
}
