<?hh

// Coercion for concat/interp always throws; run() observes each case without
// aborting the rest of the test.
function run((function(): string) $f): void {
  try {
    echo $f();
  } catch (InvalidOperationException $e) {
    echo "[throw] " . $e->getMessage() . "\n";
  }
}

<<__EntryPoint>>
function main(): void {
  echo "--- . with empty LHS ---\n";
  run(() ==> '' . 0);
  run(() ==> '' . -10);
  run(() ==> '' . 100);
  run(() ==> '' . 1.234);
  run(() ==> '' . -3.4e10);
  run(() ==> '' . INF);
  run(() ==> '' . -INF);
  run(() ==> '' . NAN);
  run(() ==> '' . true);
  run(() ==> '' . false);
  run(() ==> '' . null);
  run(() ==> '' . PHP_INT_MAX);
  run(() ==> '' . HH\stdin());
  run(() ==> '' . "string\n");

  echo "--- . with empty RHS ---\n";
  run(() ==> 0 . '');
  run(() ==> -10 . '');
  run(() ==> 100 . '');
  run(() ==> 1.234 . '');
  run(() ==> -3.4e10 . '');
  run(() ==> INF . '');
  run(() ==> -INF . '');
  run(() ==> NAN . '');
  run(() ==> true . '');
  run(() ==> false . '');
  run(() ==> null . '');
  run(() ==> PHP_INT_MAX . '');
  run(() ==> HH\stdin() . '');
  run(() ==> "string\n" . '');

  echo "--- . with non-empty LHS ---\n";
  run(() ==> '<' . 0);
  run(() ==> '<' . -10);
  run(() ==> '<' . 100);
  run(() ==> '<' . 1.234);
  run(() ==> '<' . -3.4e10);
  run(() ==> '<' . INF);
  run(() ==> '<' . -INF);
  run(() ==> '<' . NAN);
  run(() ==> '<' . true);
  run(() ==> '<' . false);
  run(() ==> '<' . null);
  run(() ==> '<' . PHP_INT_MAX);
  run(() ==> '<' . HH\stdin());
  run(() ==> '<' . "string\n");

  echo "--- concatN middle ---\n";
  run(() ==> '<' . 0 . '>');
  run(() ==> '<' . -10 . '>');
  run(() ==> '<' . 100 . '>');
  run(() ==> '<' . 1.234 . '>');
  run(() ==> '<' . -3.4e10 . '>');
  run(() ==> '<' . INF . '>');
  run(() ==> '<' . -INF . '>');
  run(() ==> '<' . NAN . '>');
  run(() ==> '<' . true . '>');
  run(() ==> '<' . false . '>');
  run(() ==> '<' . null . '>');
  run(() ==> '<' . PHP_INT_MAX . '>');
  run(() ==> '<' . HH\stdin() . '>');
  run(() ==> '<' . "string\n" . '>');

  echo "--- concatN left ---\n";
  run(() ==> 0 . ' string' . "\n");
  run(() ==> -10 . ' string' . "\n");
  run(() ==> 100 . ' string' . "\n");
  run(() ==> 1.234 . ' string' . "\n");
  run(() ==> -3.4e10 . ' string' . "\n");
  run(() ==> INF . ' string' . "\n");
  run(() ==> -INF . ' string' . "\n");
  run(() ==> NAN . ' string' . "\n");
  run(() ==> true . ' string' . "\n");
  run(() ==> false . ' string' . "\n");
  run(() ==> null . ' string' . "\n");
  run(() ==> PHP_INT_MAX . ' string' . "\n");
  run(() ==> HH\stdin() . ' string' . "\n");
  run(() ==> "string" . ' string' . "\n");

  echo "--- concatN emptystring left ---\n";
  run(() ==> '' . 0 . "\n");
  run(() ==> '' . -10 . "\n");
  run(() ==> '' . 100 . "\n");
  run(() ==> '' . 1.234 . "\n");
  run(() ==> '' . -3.4e10 . "\n");
  run(() ==> '' . INF . "\n");
  run(() ==> '' . -INF . "\n");
  run(() ==> '' . NAN . "\n");
  run(() ==> '' . true . "\n");
  run(() ==> '' . false . "\n");
  run(() ==> '' . null . "\n");
  run(() ==> '' . PHP_INT_MAX . "\n");
  run(() ==> '' . HH\stdin() . "\n");
  run(() ==> '' . "string" . "\n");

  echo "--- concatN emptystring right ---\n";
  run(() ==> 'value:' . 0 . '');
  run(() ==> 'value:' . -10 . '');
  run(() ==> 'value:' . 100 . '');
  run(() ==> 'value:' . 1.234 . '');
  run(() ==> 'value:' . -3.4e10 . '');
  run(() ==> 'value:' . INF . '');
  run(() ==> 'value:' . -INF . '');
  run(() ==> 'value:' . NAN . '');
  run(() ==> 'value:' . true . '');
  run(() ==> 'value:' . false . '');
  run(() ==> 'value:' . null . '');
  run(() ==> 'value:' . PHP_INT_MAX . '');
  run(() ==> 'value:' . HH\stdin() . '');
  run(() ==> 'value:' . "string" . '');
  echo "\n";

  echo "--- .= (string LHS, coerce RHS) ---\n";
  run(() ==> { $a = ''; $a .= 0;           return "$a\n"; });
  run(() ==> { $a = ''; $a .= -10;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= 100;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= 1.234;       return "$a\n"; });
  run(() ==> { $a = ''; $a .= -3.4e10;     return "$a\n"; });
  run(() ==> { $a = ''; $a .= INF;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= -INF;        return "$a\n"; });
  run(() ==> { $a = ''; $a .= NAN;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= true;        return "$a\n"; });
  run(() ==> { $a = ''; $a .= false;       return "$a\n"; });
  run(() ==> { $a = ''; $a .= null;        return "$a\n"; });
  run(() ==> { $a = ''; $a .= PHP_INT_MAX; return "$a\n"; });
  run(() ==> { $a = ''; $a .= HH\stdin();  return "$a\n"; });
  run(() ==> { $a = ''; $a .= "string";    return "$a\n"; });

  echo "--- .= (coerce LHS, string RHS) ---\n";
  run(() ==> { $a = 0;           $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = -10;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = 100;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = 1.234;       $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = -3.4e10;     $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = INF;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = -INF;        $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = NAN;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = true;        $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = false;       $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = null;        $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = PHP_INT_MAX; $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = HH\stdin();  $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = "string";    $a .= 'foo'; return "$a\n"; });
}
