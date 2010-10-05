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

$ret = '<?php ';
foreach ($funcs as $func) {
  $info = phpnet_get_function_info($func);

  $ret .=
    'DefineFunction(array("name" => "' . $func . '", '.
    '  "desc" => "'. escape_php(idx($info, 'desc')) .'", '.
    '  "flags"  =>  HasDocComment,'.
    '  "return" => array('.
    '    "type"   => Variant,'.
    '    "desc"   => "'. escape_php(idx($info, 'ret')) .'",'.
    '  ),'.
    '  "args"   => array(';

  if (isset($info['params'])) {
    for ($i = 0; $i < count($info['params']); $i++) {
      $ret .=
        '    array('.
        '      "name"   => "' . $info['param_names'][$i] . '",'.
        '      "type"   => Variant,'.
        '      "desc"   => "'. escape_php($info['params'][$i]) .'",'.
        '    ),';
    }
  }

  $ret .=
    '  ),'.
    '));';
}

file_put_contents("$name.idl.php", $ret);
