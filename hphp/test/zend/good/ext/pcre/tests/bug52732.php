<?php <<__EntryPoint>> function main() {
$ret = preg_match('/(?:\D+|<\d+>)*[!?]/', 'foobar foobar foobar');

var_dump($ret);
}
