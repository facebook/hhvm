<?php


<<__EntryPoint>>
function main_1234() {
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
}
