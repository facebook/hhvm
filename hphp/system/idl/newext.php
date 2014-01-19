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

function get_func_info($func, $clsname = 'function') {
  $info = phpnet_get_function_info($func, $clsname);

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

  return $arr;
}


$ret = array(
  'preamble' => '',
  'consts' => array(),
  'funcs' => array(),
  'classes' => array(),
);

$funcs = phpnet_get_extension_functions($name);
foreach ($funcs as $func) {
  print "Importing $func\n";
  $ret['funcs'][] = get_func_info($func);
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

$classes = phpnet_get_extension_classes($name);
foreach ($classes as $class) {
  print "Importing $class\n";
  $info = phpnet_get_class_info($class);

  $arr = array(
    'name' => $class,
    'desc' => idx($info, 'desc'),
    'flags' => array(
      'ZendCompat',
    ),
    'parent' => idx($info, 'parent'),
    'funcs' => array()
  );

  foreach (array_unique($info['funcs']) as $func) {
    print "Importing $class::$func\n";
    $arr['funcs'][] = get_func_info($func, $class);
  }

  $ret['classes'][] = $arr;
}

file_put_contents("$name.idl.json", json_encode($ret, JSON_PRETTY_PRINT));
