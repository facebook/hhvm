#!/usr/bin/env hhvm
<?php
// Copyright 2004-present Facebook. All Rights Reserved.

require(__DIR__.'/perf-lib.php');

function usage($script_name) {
  echo <<<EOT
Usage:

perf script -f comm,ip,sym | $script_name

EOT;
}

function starts_with($str, $prefix) {
  return strncmp($str, $prefix, strlen($prefix)) === 0;
}

# If $stack has any frames in the translation cache, return the C++ frame
# directly called by the deepest translation cache frame. If $stack has no TC
# frames, return null.
function tc_callee($stack) {
  for ($i = 0; $i < count($stack); ++$i) {
    // Normal translations starts with PHP::. Unique stubs start with HHVM::.
    if (!starts_with($stack[$i], 'PHP::') &&
        !starts_with($stack[$i], 'HHVM::')) {
      continue;
    }

    while (--$i >= 0) {
      if ($stack[$i] === '[unknown]') continue;
      return $stack[$i];
    }
    return '<unknown>';
  }
  return null;
}

# If $container is an int, return it. Otherwise, assume $container is a
# Map<string, int> and return the sum of its values.
function count_leaves($container) {
  if (is_int($container)) return $container;

  $count = 0;
  foreach ($container as $value) {
    $count += $value;
  }
  return $count;
}

# Print a summary of the data in $map, using keys as categories and values
# as sample counts. If the values in $map are more Maps, sample counts will be
# obtained by counting leaves, allowing for multiple levels of categorization.
function print_map($header, $map, $total) {
  printf("%s\n", $header);
  uasort($map, ($a, $b) ==> count_leaves($b) - count_leaves($a));

  foreach ($map as $name => $samples) {
    $count = count_leaves($samples);
    $pct_total = $count * 100.0 / $total;
    if ($pct_total < 0.10) break;
    printf("  %5.2f%% - %s\n", $pct_total, $name);
  }
  printf("\n");
}

# Increment $map[$key] by 1, setting it to 0 first if it doesn't exist.
function increment_key($map, $key) {
  if (!isset($map[$key])) $map[$key] = 0;
  $map[$key]++;
}

# Increment $map[$key1][$key2] by 1, setting it to 0 first if it doesn't exist.
function increment_key2($map, $key1, $key2) {
  if (!isset($map[$key1])) $map[$key1] = Map {};
  increment_key($map[$key1], $key2);
}

function categorize_helper($func) {
  // Order is important in this Map: we return the first category with a match,
  // even though a later category may also match.
  static $categories = Map {
    'InterpOne' => Vector {
      '/interpOne/',
    },
    'Iterators' => Vector {
      '/iter/i',
    },
    'Async' => Vector {
      '/WaitHandle/',
      '/AsioBlockableChain/',
    },
    'CacheClient' => Vector {
      '/c_CacheClient2/',
    },
    'Arrays' => Vector {
      '/Elem/i',
      '/::array/',
      '/(Mixed|Packed)Array/',
      '/^HPHP::f_(.?.?sort|array)/',
      '/NameValueTable/',
      '/jit::dict/',
    },
    'Objects' => Vector {
      '/::ObjectData::/',
      '/initSProps/',
      '/MethodCache/',
      '/lookupClsMethodHelper/',
      '/newInstance/',
      '/Prop/',
      '/::closureInstance.tor/',
    },
    'Strings' => Vector {
      '/::concat/',
      '/::StringData::/',
      '/^HPHP::f_(explode|join|str|mb_strtolower|substr|preg_)/',
      '/^HPHP::conv_10/',
    },
    'OBC' => Vector {
      '/::ObcStore::/',
      '/^HPHP::f_obc_/',
    },
    'Extensions' => Vector {
      '/^HPHP::f_/',
      '/^HPHP::thrift::f_/',
      '/^HPHP::math_mt_rand/',
    },
    'Collections' => Vector {
      '/deepInitHelper/',
      '/Vector/',
      '/mapSetImpl/',
      '/Map/',
      '/Set/',
      '/ImmSet/',
      '/HashCollection/',
    },
  };

  foreach ($categories as $cat => $regexes) {
    foreach ($regexes as $regex) {
      if (preg_match($regex, $func) === 1) return $cat;
    }
  }
  return 'Uncategorized';
}

function main($argv) {
  ini_set('memory_limit', '64G');
  if (posix_isatty(STDIN)) {
    usage($argv[0]);
    return 1;
  }

  fprintf(STDERR, ">> Parsing samples from stdin...\n");
  $samples = read_perf_samples(STDIN);
  $groups = Map {};
  $helpers = Map {};

  fprintf(STDERR, ">> Categorizing samples...\n");
  foreach ($samples as $stack) {
    if (starts_with($stack[0], 'PHP::')) {
      increment_key($groups, 'TC');
    } else if (starts_with($stack[0], 'HHVM::pcre_jit::')) {
      increment_key($groups, 'PCRE JIT');
    } else if ($callee = tc_callee($stack)) {
      increment_key($groups, 'TC helpers');
      $category = categorize_helper($callee);
      increment_key2($helpers, $category, $callee);
    } else {
      increment_key($groups, 'Other');
    }
  }

  $total_samples = count($samples);
  print_map('Summary', $groups, $total_samples);
  print_map('Helper categories', $helpers, $total_samples);
  foreach ($helpers as $category => $map) {
    print_map($category, $map, $total_samples);
  }
}

exit(main($argv));
