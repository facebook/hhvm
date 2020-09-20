<?hh
<<__EntryPoint>> function main(): void {
$long_max = is_int(5000000000)? (float)9223372036854775807 : (float)0x7FFFFFFF;
$long_min = -$long_max - 1;
printf("%d,%d,%d,%d\n",is_float($long_min  ),is_float($long_max  ),
                       is_int($long_min-1),is_int($long_max+1));

include(dirname(__FILE__) . '/../../../../tests/quicktester.inc');
quicktester(() ==> 1, () ==> abs(-1));
quicktester(() ==> 1.5, () ==> abs(-1.5));
quicktester(() ==> 1, () ==> abs("-1"));
quicktester(() ==> 1.5, () ==> abs("-1.5"));
quicktester(() ==> -($long_min+1), () ==> abs($long_min-1));
quicktester(() ==> -($long_min), () ==> abs($long_min));
quicktester(() ==> -($long_min+1), () ==> abs($long_min+1));
}
