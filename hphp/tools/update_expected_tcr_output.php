#!/bin/env php
<?php

$php = `which hhvm`;
if (!$php) $php = `which php`;
$config = json_decode(file_get_contents('tools/update_expected_tcr_output.json'), true);

function ends_with($big, $little) {
  return strpos($big, $little, strlen($big) - strlen($little)) !== false;
}

// More efficient lookups than array_search
foreach ($config as $key => &$value) {
  $value = array_fill_keys($value, true);
}

foreach (new RecursiveIteratorIterator (new RecursiveDirectoryIterator ('test/tcr')) as $f) {
  $filename = $f->getRealPath();
  $name = str_replace('.php', '', $f->getFilename());

  if (!$f->isFile() ||
      !ends_with($filename, '.php') ||
      isset($config['no_zend'][$name])) {
    continue;
  }

  $opts = '-dapc.enable_cli=1 -ddisplay_errors=off';
  `$php $opts $filename > $filename.expect`;
}
