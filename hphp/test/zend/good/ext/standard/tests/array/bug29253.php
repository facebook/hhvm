<?hh
<<__EntryPoint>> function main(): void {
$zz = $GLOBALS;
$gg = 'afad';
var_dump(@array_diff_assoc($GLOBALS, $zz));
var_dump($gg);
}
