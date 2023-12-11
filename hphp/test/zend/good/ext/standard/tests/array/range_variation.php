<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing range() with various low and high values ***";
$low_arr = vec[ "ABCD", -10.5555, TRUE, NULL, FALSE, "", vec[1,2]];
$high_arr = vec[ "ABCD", -10.5555, TRUE, NULL, FALSE, "", vec[1,2]];

for( $i = 0; $i < count($low_arr); $i++) {
  for( $j = 0; $j < count($high_arr); $j++) {
    $tl = HH\is_any_array($low_arr[$i]) ? 'Array' : $low_arr[$i];
    $th = HH\is_any_array($high_arr[$j]) ? 'Array' : $high_arr[$j];
    echo @"\n-- creating an array with low = '$tl' and high = '$th' --\n";
    var_dump( range( $low_arr[$i], $high_arr[$j] ) );
  }
}

echo "\n*** Possible variatins with steps ***\n";
var_dump( range( 1, 5, TRUE ) );
var_dump( range( 1, 5, vec[1, 2] ) );

echo "Done\n";
}
