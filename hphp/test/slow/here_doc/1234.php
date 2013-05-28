<?php

$nullherequote= <<<fail
fail;
echo "--$nullherequote--\n";
$x="foo";
$threestops= <<<pass
passable $x
pass;x
ss;
pass;
echo "$threestops\n";
