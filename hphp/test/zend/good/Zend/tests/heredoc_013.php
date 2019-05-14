<?php <<__EntryPoint>> function main() {
$test = "foo";
$var = prefix<<<"MYLABEL"
test: $test
MYLABEL;
echo $var;
}
