<?hh

class Counter {
  public static int $count1 = 0;
  public static int $count2 = 0;
}

<<__Memoize(#KeyedByIC)>>
function keyed_memo(): string {
  Counter::$count1++;
  $key = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo "  keyed_memo EXECUTED (run #" . Counter::$count1 . ", memo_key=" . $key . ")\n";
  return "result_" . Counter::$count1 . '_key_'. $key;
}

<<__Memoize(#KeyedByIC)>>
function keyed_memo2(): string {
  Counter::$count2++;
  $key = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
  echo "  keyed_memo2 EXECUTED (run #" . Counter::$count2 . ", memo_key=" . $key . ")\n";
  return "result2_" . Counter::$count2 . '_key_'. $key;
}

function call_memo(string $label): void {
  echo $label . ":\n";
  $result = keyed_memo();
  echo "  returned: " . $result . "\n";
}

function call_memo2(string $label): void {
  echo $label . ":\n";
  $result = keyed_memo2();
  echo "  returned: " . $result . "\n";
}

// Test 1: Validate that unsetting a sensitive context produces the same
// memo state as the backdoor (inaccessible) state.
// A <<__Memoize(#KeyedByIC)>> function is called from both states;
// only one execution should occur since they share the same memo key.
function test_unset_matches_backdoor(): void {
  echo "=== Unset produces same memo state as backdoor ===\n";

  // 1) Call from no-context state (memo key 0) - baseline, cache miss
  call_memo("No context (baseline)");

  MemoSensitiveIntCtx::start(new Base(42), () ==> {
    // 2) Call with sensitive context set - different memo key, cache miss
    call_memo("With sensitive context");

    // 3) Unset the sensitive context - should revert to memo key 0, cache hit
    MemoSensitiveIntCtx::start(null, () ==> {
      call_memo("After unset sensitive");
      return 0;
    });

    // 4) Call via backdoor - should also be memo key 0, cache hit
    HH\Coeffects\backdoor(() ==> {
      call_memo("Via backdoor");
    });

    return 0;
  });

  echo "\nTotal keyed_memo executions: " . Counter::$count1 . "\n";
}

// Test 2: Agnostic context changes don't affect the memo key.
// Setting or unsetting agnostic context while a sensitive context is active
// should not cause additional memo cache misses.
function test_agnostic_unset_memo(): void {
  echo "\n=== Agnostic set/unset doesn't affect memo key ===\n";

  MemoSensitiveIntCtx::start(new Base(42), () ==> {
    // Call with sensitive context - cache hit from test 1 (same sensitive key)
    call_memo2("With sensitive context");

    // Set agnostic, call memo - cache hit (agnostic doesn't affect memo)
    MemoAgnosticIntCtx::start(10, () ==> {
      call_memo2("With sensitive + agnostic set");

      // Unset agnostic, call memo - still cache hit
      MemoAgnosticIntCtx::start(null, () ==> {
        call_memo2("With sensitive + agnostic unset");
        return 0;
      });
      return 0;
    });
    return 0;
  });

  echo "\nTotal keyed_memo2 executions: " . Counter::$count2 . "\n";
}

<<__EntryPoint>>
function main(): mixed {
  include 'unset-context.inc';
  test_unset_matches_backdoor();
  test_agnostic_unset_memo();
}
