<?php
<<__EntryPoint>> function main() {
$arr = array('strtoupper', 'strtolower');

$k = 0;

var_dump($arr[0]('foo') == 'FOO');
var_dump($arr[$k]('foo') == 'FOO');
var_dump($arr[++$k]('FOO') == 'foo');
try { var_dump($arr[++$k]('FOO') == 'foo'); }
catch (Exception $e) { echo $e->getMessage()."\n"; }
}
