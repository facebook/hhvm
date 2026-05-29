<?hh

function printState(string $label): void {
  echo "$label:\n";
  echo "  MemoAgnostic exists: " .
    (MemoAgnosticIntCtx::hasContext() ? 'true' : 'false') . "\n";
  echo "  MemoAgnostic get: ";
  var_dump(MemoAgnosticIntCtx::getContext());
  echo "  MemoSensitive exists: " .
    (MemoSensitiveIntCtx::hasContext() ? 'true' : 'false') . "\n";
  $c = MemoSensitiveIntCtx::getContext();
  echo "  MemoSensitive get: ";
  var_dump($c ? $c->getPayload() : null);
}

// Test 1: Set agnostic, then unset it
function test_agnostic_unset(): void {
  echo "=== Test 1: Agnostic set then unset ===\n";
  MemoAgnosticIntCtx::start(42, () ==> {
    printState("After set");
    MemoAgnosticIntCtx::start(null, () ==> {
      printState("After unset");
      return 0;
    });
    printState("After unset scope");
    return 0;
  });
}

// Test 2: Set sensitive, then unset it
function test_sensitive_unset(): void {
  echo "=== Test 2: Sensitive set then unset ===\n";
  MemoSensitiveIntCtx::start(new Base(99), () ==> {
    printState("After set");
    MemoSensitiveIntCtx::start(null, () ==> {
      printState("After unset");
      return 0;
    });
    printState("After unset scope");
    return 0;
  });
}

// Test 3: Set both, unset agnostic only
function test_unset_agnostic_keeps_sensitive(): void {
  echo "=== Test 3: Set both, unset agnostic only ===\n";
  MemoAgnosticIntCtx::start(10, () ==> {
    MemoSensitiveIntCtx::start(new Base(20), () ==> {
      printState("Both set");
      MemoAgnosticIntCtx::start(null, () ==> {
        printState("Agnostic unset, sensitive remains");
        return 0;
      });
      return 0;
    });
    return 0;
  });
}

// Test 4: Set both, unset sensitive only
function test_unset_sensitive_keeps_agnostic(): void {
  echo "=== Test 4: Set both, unset sensitive only ===\n";
  MemoAgnosticIntCtx::start(10, () ==> {
    MemoSensitiveIntCtx::start(new Base(20), () ==> {
      printState("Both set");
      MemoSensitiveIntCtx::start(null, () ==> {
        printState("Sensitive unset, agnostic remains");
        return 0;
      });
      return 0;
    });
    return 0;
  });
}

// Test 5: Unset without prior set (noop)
function test_unset_without_set(): void {
  echo "=== Test 5: Unset without prior set ===\n";
  MemoAgnosticIntCtx::start(null, () ==> {
    printState("Agnostic unset (was never set)");
    return 0;
  });
  MemoSensitiveIntCtx::start(null, () ==> {
    printState("Sensitive unset (was never set)");
    return 0;
  });
}

<<__EntryPoint>>
function main(): mixed {
  include 'unset-context.inc';
  test_agnostic_unset();
  test_sensitive_unset();
  test_unset_agnostic_keeps_sensitive();
  test_unset_sensitive_keeps_agnostic();
  test_unset_without_set();
}
