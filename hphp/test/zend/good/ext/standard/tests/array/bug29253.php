<?hh
<<__EntryPoint>> function main(): void {
$zz = $GLOBALS['GLOBALS'];
$gg = 'afad';
var_dump(@array_diff_assoc($GLOBALS['GLOBALS'], $zz));
var_dump($gg);
}
