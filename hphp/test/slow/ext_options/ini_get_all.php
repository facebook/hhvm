<?php

$all_detailed = ini_get_all();
var_dump($all_detailed['hphp.compiler_version']['access']);
var_dump($all_detailed['allow_url_fopen']);
var_dump($all_detailed['arg_separator.output']);

$all_short = ini_get_all(null, false);
var_dump($all_short['allow_url_fopen']);
var_dump($all_short['arg_separator.output']);

var_dump(ini_get_all('pcre'));
var_dump(ini_get_all('pcre', false));

$core = ini_get_all('core');
var_dump(array(
  'core: allow_url_fopen' => isset($core['allow_url_fopen']),
  'core: pcre.backtrack_limit' => isset($core['pcre.backtrack_limit']),
));

ini_get_all("THIS_EXTENSION_SHOULD_NOT_EXIST");
