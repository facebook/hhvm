<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function starts_with($str, $prefix) {
  return strncmp($str, $prefix, strlen($prefix)) === 0;
}

<<__Memoize>>
function filter_func_raw(string $func): string {
  return trim(shell_exec("echo '$func' | c++filt"));
}

// If $func looks like a mangled C++ symbol, attempt to demangle it, stripping
// off any trailing junk first.
function filter_func(string $func): string {
  if (strncmp($func, '_Z', 2) === 0) {
    $matches = null;
    if (
      preg_match_with_matches('/^(.+)\.isra\.\d+$/', $func, inout $matches) ===
        1
    ) {
      $func = $matches[1];
    }
    return filter_func_raw($func);
  }

  return $func;
}

// Read perf samples from the given file stream into a Vector of stack traces.
// The stream should contain the output of "perf script -f comm,ip,sym".
function read_perf_samples($file, $desired_binary_prefix = 'hhvmworker') {
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

    $matches = null;
    if (
      preg_match_with_matches('/^[a-f0-9]+ (.+)$/', $line, inout $matches) === 1
    ) {
      if (!$stack) $stack = Vector {};
      $stack[] = filter_func($matches[1]);
    } else {
      if ($stack !== null) throw new Exception("Unexpected line $line");
      $binary = $line;
      $skip_sample = !starts_with($binary, $desired_binary_prefix);
    }
  }

  if ($stack) $samples[] = $stack;
  return $samples;
}
