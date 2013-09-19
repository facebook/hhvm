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

$ret = array(
  'preamble' => '',
  'consts' => array(),
  'funcs' => array(),
  'classes' => array(),
);

$funcs = phpnet_get_extension_functions($name);
foreach ($funcs as $func) {
  $info = phpnet_get_function_info($func);

  $arr = array(
    'name' => $func,
    'desc' => idx($info, 'desc'),
    'flags' => array(
      'ZendCompat',
      'NeedsActRec',
    ),
    'return' => array(
      'type' => 'Variant',
      'desc' => idx($info, 'ret'),
    )
  );

  $args = array();
  if (isset($info['params'])) {
    for ($i = 0; $i < count($info['params']); $i++) {
      $args[] = array(
        'name' => $info['param_names'][$i],
        'type' => 'Variant',
        'desc' => $info['params'][$i],
      );
    }
  }
  $arr['args'] = $args;

  $ret['funcs'][] = $arr;
}

$consts = phpnet_get_extension_constants($name);
foreach ($consts as $const) {
  if (!defined($const)) {
    var_dump("Undefined: $const. Try using Zend to run this script");
    continue;
  }
  $ret['consts'][] = array(
    'name' => $const,
    'value' => constant($const),
  );
}

file_put_contents("$name.idl.json", json_encode($ret, JSON_PRETTY_PRINT));
