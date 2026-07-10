<?hh

// Only int/string literal operands remain; run() surfaces any regressed coercion as [throw].
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
  run(() ==> '' . PHP_INT_MAX);
  run(() ==> '' . "string\n");

  echo "--- . with empty RHS ---\n";
  run(() ==> 0 . '');
  run(() ==> -10 . '');
  run(() ==> 100 . '');
  run(() ==> PHP_INT_MAX . '');
  run(() ==> "string\n" . '');

  echo "--- . with non-empty LHS ---\n";
  run(() ==> '<' . 0);
  run(() ==> '<' . -10);
  run(() ==> '<' . 100);
  run(() ==> '<' . PHP_INT_MAX);
  run(() ==> '<' . "string\n");

  echo "--- concatN middle ---\n";
  run(() ==> '<' . 0 . '>');
  run(() ==> '<' . -10 . '>');
  run(() ==> '<' . 100 . '>');
  run(() ==> '<' . PHP_INT_MAX . '>');
  run(() ==> '<' . "string\n" . '>');

  echo "--- concatN left ---\n";
  run(() ==> 0 . ' string' . "\n");
  run(() ==> -10 . ' string' . "\n");
  run(() ==> 100 . ' string' . "\n");
  run(() ==> PHP_INT_MAX . ' string' . "\n");
  run(() ==> "string" . ' string' . "\n");

  echo "--- concatN emptystring left ---\n";
  run(() ==> '' . 0 . "\n");
  run(() ==> '' . -10 . "\n");
  run(() ==> '' . 100 . "\n");
  run(() ==> '' . PHP_INT_MAX . "\n");
  run(() ==> '' . "string" . "\n");

  echo "--- concatN emptystring right ---\n";
  run(() ==> 'value:' . 0 . '');
  run(() ==> 'value:' . -10 . '');
  run(() ==> 'value:' . 100 . '');
  run(() ==> 'value:' . PHP_INT_MAX . '');
  run(() ==> 'value:' . "string" . '');
  echo "\n";

  echo "--- .= (string LHS, coerce RHS) ---\n";
  run(() ==> { $a = ''; $a .= 0;           return "$a\n"; });
  run(() ==> { $a = ''; $a .= -10;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= 100;         return "$a\n"; });
  run(() ==> { $a = ''; $a .= PHP_INT_MAX; return "$a\n"; });
  run(() ==> { $a = ''; $a .= "string";    return "$a\n"; });

  echo "--- .= (coerce LHS, string RHS) ---\n";
  run(() ==> { $a = 0;           $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = -10;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = 100;         $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = PHP_INT_MAX; $a .= 'foo'; return "$a\n"; });
  run(() ==> { $a = "string";    $a .= 'foo'; return "$a\n"; });
}
