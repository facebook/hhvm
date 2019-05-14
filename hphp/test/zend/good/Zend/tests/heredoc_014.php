<?php <<__EntryPoint>> function main() {
$test = "foo";
$var = <<<"MYLABEL
test: $test
MYLABEL;
echo $var;
}
