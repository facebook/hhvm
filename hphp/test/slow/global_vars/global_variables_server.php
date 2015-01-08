<?php

require_once(__DIR__ . '/../../server/fastcgi/tests/test_base.inc');

$DOC_ROOT = __DIR__;

function handle_request() {
  return empty($_ENV) || (
      array_key_exists('HPHP_SERVER', $_ENV) && $_ENV['HPHP_SERVER']
      ) || php_sapi_name() != 'cli';
}

if (handle_request()) {
  // Handle requests
  var_dump($HTTP_RAW_POST_DATA);
  var_dump(count($_ENV) > 0);
  var_dump($_GET);
  var_dump($_POST);
  var_dump($_COOKIE);
  var_dump(count($_SERVER) > 0);
  var_dump($_REQUEST);
  exit();
}

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
    $path = 'global_variables_server.php?var=GET&get=1';
    $post = array('var' => 'POST', 'post' => 2);
    $headers = array('Cookie' => 'var=COOKIE;cookie=3;');
    echo request(php_uname('n'), $port, $path, $post, $headers, $extra) . "\n";
  }, $request[0]);
}
