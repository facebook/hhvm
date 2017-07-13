<?php

echo "statically-known locals\n";

$x = 4;
echo $x . "\n";

echo ++$x . "\n";
echo $x++ . "\n";
echo $x . "\n";
echo --$x . "\n";
echo $x-- . "\n";
echo $x . "\n";

$x += 3;
echo $x . "\n";

$x -= 1 + 1;
echo $x . "\n";

$x *= 4;
echo $x . "\n";

$x /= 4;
echo $x . "\n";

$x %= 3;
echo $x . "\n";

$x **= 8;
echo $x . "\n";

$x |= 7;
echo $x . "\n";

$x &= 5;
echo $x . "\n";

$x ^= 128;
echo $x . "\n";

$x ^= 129;
echo $x . "\n";

$x <<= 2;
echo $x . "\n";

$x >>= 1;
echo $x . "\n";

$x .= "foo";
echo $x . "\n";

echo "\ndynamically-known locals\n";

$x = "quux";
$$x = 4;
echo $$x . "\n";
echo ${"qu" . "ux"} . "\n";
echo ${$x} . "\n";


echo ++$$x . "\n";
echo $$x++ . "\n";
echo $$x . "\n";
echo --$$x . "\n";
echo $$x-- . "\n";
echo $$x . "\n";

$$x += 3;
echo $$x . "\n";

$$x -= 1 + 1;
echo $$x . "\n";

$$x *= 4;
echo $$x . "\n";

$$x /= 4;
echo $$x . "\n";

$$x %= 3;
echo $$x . "\n";

$$x **= 8;
echo $$x . "\n";

$$x |= 7;
echo $$x . "\n";

$$x &= 5;
echo $$x . "\n";

$$x ^= 128;
echo $$x . "\n";

$$x ^= 129;
echo $$x . "\n";

$$x <<= 2;
echo $$x . "\n";

$$x >>= 1;
echo $$x . "\n";

$$x .= "foo";
echo $$x . "\n";

echo "\ncorner cases\n";

${2} = "foo";
echo ${1 + 1} . "\n";
${true} = "bar";
$true = "quux";
echo $true . "\n";
echo ${true} . "\n";
