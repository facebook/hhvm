<?php

/**
 * Generate initial IDL file from php.net. For example,
 *
 *   php newext.php imap
 *   make imap.gendoc
 *
 * Then manually correct parameter and return types.
 */

$name = $argv[1];

require 'base.php';
$funcs = phpnet_get_extension_functions($name);

$ret = array();
foreach ($funcs as $func) {
  $info = phpnet_get_function_info($func);

  $arr = array(
    'name' => $func,
    'desc' => idx($info, 'desc'),
    'flags' => 'HasDocComment',
    'return' => array(
      'type' => 'Variant',
      'desc' => idx($info, 'ret'),
    )
  );

  if (isset($info['params'])) {
    $args = array();
    for ($i = 0; $i < count($info['params']); $i++) {
      $args[] = array(
        'name' => $info['param_names'][$i],
        'type' => 'Variant',
        'desc' => $info['params'][$i],
      );
    }
    $arr['args'] = $args;
  }

  $ret[] = $arr;
}

file_put_contents("$name.idl.json", json_encode($ret, JSON_PRETTY_PRINT));
