#!/bin/env php
<?php

$php = '/home/engshare/externals/cpp/hphp/centos-dev/php/bin/php';
// $php = '/home/ptarjan/bin/php-5.4';
$config = json_decode(file_get_contents(__DIR__.'/update_expected_slow_output.json'), true);
if (!$config) {
  die("Invalid config file. Not JSON.\n");
}

function ends_with($big, $little) {
  return strpos($big, $little, strlen($big) - strlen($little)) !== false;
}

// More efficient lookups than array_search
foreach ($config as $key => &$value) {
  $value = array_fill_keys($value, true);
}

function is_valid_diff($wanted_re, $output) {
  $wanted_re = trim($wanted_re);
  $output = trim($output);
  $wanted_re = preg_quote($wanted_re, '/');

  $wanted_re = str_replace('%e', '\\' . DIRECTORY_SEPARATOR, $wanted_re);
  $wanted_re = str_replace('%s', '[^\r\n]+', $wanted_re);
  $wanted_re = str_replace('%S', '[^\r\n]*', $wanted_re);
  $wanted_re = str_replace('%a', '.+', $wanted_re);
  $wanted_re = str_replace('%A', '.*', $wanted_re);
  $wanted_re = str_replace('%w', '\s*', $wanted_re);
  $wanted_re = str_replace('%i', '[+-]?\d+', $wanted_re);
  $wanted_re = str_replace('%d', '\d+', $wanted_re);
  $wanted_re = str_replace('%x', '[0-9a-fA-F]+', $wanted_re);
  $wanted_re = str_replace('%f', '[+-]?\.?\d+\.?\d*(?:[Ee][+-]?\d+)?', $wanted_re);
  $wanted_re = str_replace('%c', '.', $wanted_re);

  return preg_match("/^$wanted_re\$/sm", $output);
};

print "Running all test/slow through $php...\n";
foreach (new RecursiveIteratorIterator (new RecursiveDirectoryIterator ('test/slow')) as $f) {
  $filename = $f->getRealPath();
  $name = str_replace('.php', '', $f->getFilename());

  if (!$f->isFile() ||
      !ends_with($filename, '.php') ||
      isset($config['hard_coded'][$name])) {
    continue;
  }

  $opts = '-dapc.enable_cli=1 -ddisplay_errors=off';
  $output = shell_exec("$php $opts $filename 2>/dev/null");

  $expect = "$filename.expect";
  if (is_file($expect)) {
    file_put_contents($expect, $output);
  } else if (is_file($expect.'f')) {
    $wanted_re = file_get_contents($expect.'f');
    if (isset($config['no_warnings'][$name])) {
      $wanted_re = preg_replace('/^HipHop .*/m', '', $wanted_re);
    }
    $output = str_replace(realpath(__DIR__.'../../..'), '%s', $output);
    if (is_valid_diff($wanted_re, $output)) {
      continue;
    }

    $file = tempnam('/tmp', 'zend-');
    file_put_contents($file, $output);
    $diff = shell_exec("diff --text -u {$expect}f $file");
    echo "\n$diff\n";

    do {
      print "Output differs, use Zend? [y/N]? ";
      $prompt = strtolower(trim(fgets(STDIN)));
      if ($prompt == 'y') {
        file_put_contents($expect.'f', $output);
        break;
      } else if (!$prompt || $prompt == 'n') {
        break;
      }
    } while (true);
  } else {
    die("No $file.expect or $file.expectf. WTF?\n");
  }
}
