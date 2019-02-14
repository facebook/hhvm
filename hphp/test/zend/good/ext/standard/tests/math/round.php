<?php // $Id$

$long_max = is_int(5000000000)? 9223372036854775807 : 0x7FFFFFFF;
$long_min =  -$long_max - 1;
printf("%d,%d,%d,%d\n",is_int($long_min  ),is_int($long_max  ),
					   is_int($long_min-1),is_int($long_max+1));

$tests = <<<TESTS
-1 ~== ceil(-1.5)
 2 ~== ceil( 1.5)
-2 ~== floor(-1.5)
 1 ~== floor(1.5)
 $long_min   ~== ceil($long_min - 0.5)
 $long_min+1 ~== ceil($long_min + 0.5)
 $long_min-1 ~== round($long_min - 0.6)
 $long_min   ~== round($long_min - 0.4)
 $long_min   ~== round($long_min + 0.4)
 $long_min+1 ~== round($long_min + 0.6)
 $long_min-1 ~== floor($long_min - 0.5)
 $long_min   ~== floor($long_min + 0.5)
 $long_max   ~== ceil($long_max - 0.5)
 $long_max+1 ~== ceil($long_max + 0.5)
 $long_max-1 ~== round($long_max - 0.6)
 $long_max   ~== round($long_max - 0.4)
 $long_max   ~== round($long_max + 0.4)
 $long_max+1 ~== round($long_max + 0.6)
 $long_max-1 ~== floor($long_max - 0.5)
 $long_max   ~== floor($long_max + 0.5)
TESTS;

include(dirname(__FILE__) . '/../../../../tests/quicktester.inc');
