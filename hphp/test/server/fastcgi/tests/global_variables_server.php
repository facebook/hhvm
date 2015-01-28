<?php

require_once(__DIR__ . '/test_base.inc');

$requests = array(
  array(
    '-dalways_populate_raw_post_data=1',
    ['CONTENT_TYPE' => 'multipart/form-data; boundary=dumy']),
  array('-dalways_populate_raw_post_data=1', []),
  array('', []),
  array('-dvariables_order=NONE -drequest_order=', []),
  array('-dvariables_order=E -drequest_order=GPC', []),
  array('-dvariables_order=CGP -drequest_order=GP', []),
  array('-dvariables_order=GC -drequest_order=CG', []),
  array('-dvariables_order=GC -drequest_order=GC', []),
  array('-dvariables_order=GC -drequest_order=P', []),
);

foreach($requests as $request) {
  echo "------------ {$request[0]} --------\n";
  runTest(function($port) use($request) {
    list($options, $extra) = $request;
    $path = 'global_variables.php?var=GET&get=1';
    $post = array('var' => 'POST', 'post' => 2);
    $headers = array('Cookie' => 'var=COOKIE;cookie=3;');
    echo request(php_uname('n'), $port, $path, $post, $headers, $extra) . "\n";
  }, $request[0]);
}
