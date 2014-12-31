<?php

$a = gmp_init(42);
$b = gmp_init(17);

var_dump($a + $b);
var_dump($a + 17);
var_dump(42 + $b);

var_dump($a - $b);
var_dump($a - 17);
var_dump(42 - $b);

var_dump($a * $b);
var_dump($a * 17);
var_dump(42 * $b);

var_dump($a / $b);
var_dump($a / 17);
var_dump(42 / $b);
var_dump($a / 0);

var_dump($a % $b);
var_dump($a % 17);
var_dump(42 % $b);
var_dump($a % 0);

var_dump($a ** $b);
var_dump($a ** 17);
var_dump(42 ** $b);

var_dump($a | $b);
var_dump($a | 17);
var_dump(42 | $b);

var_dump($a & $b);
var_dump($a & 17);
var_dump(42 & $b);

var_dump($a ^ $b);
var_dump($a ^ 17);
var_dump(42 ^ $b);

var_dump($a << $b);
var_dump($a << 17);
var_dump(42 << $b);

var_dump($a >> 2);
var_dump(-$a >> 2);

var_dump($a << -1);
var_dump($a >> -1);

var_dump(~$a);
var_dump(-$a);
var_dump(+$a);

var_dump($a == $b);
var_dump($a != $b);
var_dump($a < $b);
var_dump($a <= $b);
var_dump($a > $b);
var_dump($a >= $b);

var_dump($a == $a);
var_dump($a != $a);

var_dump($a == 42);
var_dump($a != 42);
var_dump($a < 42);
var_dump($a <= 42);
var_dump($a > 42);
var_dump($a >= 42);

var_dump($a == new stdClass);

$a += 1;
var_dump($a);
$a -= 1;
var_dump($a);

var_dump(++$a);
var_dump($a++);
var_dump($a);

var_dump(--$a);
var_dump($a--);
var_dump($a);

// Test operator that was not overloaded

var_dump($a . $b);
var_dump($a . '17');
var_dump('42' . $b);

$a .= '17';
var_dump($a);

?>
