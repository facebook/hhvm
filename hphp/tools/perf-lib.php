<?php
// Copyright 2004-present Facebook. All Rights Reserved.

# If $func looks like a mangled C++ symbol, attempt to demangle it, stripping
# off any trailing junk first.
function filter_func(string $func): string {
  static $cache = Map {};

  if (strncmp($func, '_Z', 2) === 0) {
    if (preg_match('/^(.+)\.isra\.\d+$/', $func, $matches) === 1) {
      $func = $matches[1];
    }
    if (!isset($cache[$func])) {
      $cache[$func] = trim(shell_exec("echo '$func' | c++filt"));
    }
    $func = $cache[$func];
  }

  return $func;
}

# Read perf samples from the given file stream into a Vector of stack traces.
# The stream should contain the output of "perf script -f comm,ip,sym".
function read_perf_samples($file, $desired_binary = 'hhvm') {
  $samples = Vector {};
  $skip_sample = false;
  $stack = null;
  $binary = null;

  while ($line = fgets($file)) {
    $line = trim($line);

    if ($line === '') {
      if ($stack) {
        $samples[] = $stack;
        $stack = null;
      }
      $skip_sample = false;
      continue;
    }
    if ($skip_sample) {
      continue;
    }

    if (preg_match('/^[a-f0-9]+ (.+)$/', $line, $matches) === 1) {
      if (!$stack) $stack = Vector {};
      $stack[] = filter_func($matches[1]);
    } else {
      if ($stack !== null) throw new Exception("Unexpected line $line");
      $binary = $line;
      $skip_sample = $binary !== $desired_binary;
    }
  }

  if ($stack) $samples[] = $stack;
  return $samples;
}
