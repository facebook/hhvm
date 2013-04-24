<?php
$a = $b = $c = array(0,1,2,3,4,5);
print_r($a);
// this is ok:
print_r(array_slice($a,2,2147483645));

// this is wrong:
print_r(array_slice($a,2,2147483646));
echo 'print_r(array_splice($a,2,1));'."\n";
print_r(array_splice($a,2,1));
echo "\$a is :";
print_r($a);
echo 'print_r(array_splice($b,2,2147483645));'."\n";
print_r(array_splice($b,2,2147483645));
echo "\$b is :";
print_r($b);

// this is wrong:
echo 'print_r(array_splice($c,2,2147483646));'."\n";
print_r(array_splice($c,2,2147483646));
echo "\$c is :";
print_r($c);
?>