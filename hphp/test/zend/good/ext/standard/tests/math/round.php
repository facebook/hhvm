<?hh
<<__EntryPoint>> function main(): void {
$long_max = PHP_INT_MAX;
$long_min = PHP_INT_MIN;

include(dirname(__FILE__) . '/../../../../tests/quicktester.inc');

quicktester(() ==> -1, () ==> ceil(-1.5), '~==');
quicktester(() ==>  2, () ==> ceil( 1.5), '~==');
quicktester(() ==> -2, () ==> floor(-1.5), '~==');
quicktester(() ==>  1, () ==> floor(1.5), '~==');
quicktester(() ==>  $long_min  , () ==> ceil($long_min - 0.5), '~==');
quicktester(() ==>  $long_min+1, () ==> ceil($long_min + 0.5), '~==');
quicktester(() ==>  $long_min-1, () ==> round($long_min - 0.6), '~==');
quicktester(() ==>  $long_min  , () ==> round($long_min - 0.4), '~==');
quicktester(() ==>  $long_min  , () ==> round($long_min + 0.4), '~==');
quicktester(() ==>  $long_min+1, () ==> round($long_min + 0.6), '~==');
quicktester(() ==>  $long_min-1, () ==> floor($long_min - 0.5), '~==');
quicktester(() ==>  $long_min  , () ==> floor($long_min + 0.5), '~==');
quicktester(() ==>  $long_max  , () ==> ceil($long_max - 0.5), '~==');
quicktester(() ==>  $long_max+1, () ==> ceil($long_max + 0.5), '~==');
quicktester(() ==>  $long_max-1, () ==> round($long_max - 0.6), '~==');
quicktester(() ==>  $long_max  , () ==> round($long_max - 0.4), '~==');
quicktester(() ==>  $long_max  , () ==> round($long_max + 0.4), '~==');
quicktester(() ==>  $long_max+1, () ==> round($long_max + 0.6), '~==');
quicktester(() ==>  $long_max-1, () ==> floor($long_max - 0.5), '~==');
quicktester(() ==>  $long_max  , () ==> floor($long_max + 0.5), '~==');
}
