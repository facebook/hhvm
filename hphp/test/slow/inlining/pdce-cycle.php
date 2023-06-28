<?hh

class Counter { public static int $STATE = 1; public static bool $CYCLE = true; }

<<__NEVER_INLINE>> function update(string $caller): void {
  var_dump($caller."->".__FUNCTION__);
}

<<__NEVER_INLINE>> function break_trace(): void { throw new Exception(); }

<<__ALWAYS_INLINE>>
function while_loop(): void {
  $RAN_LOOP = 0;
  while (Counter::$STATE === 1 || Counter::$STATE === 2) {
    update(__FUNCTION__);
    Counter::$STATE = Counter::$STATE - 1;
    $RAN_LOOP = 1;
  }
  if ($RAN_LOOP === 1 || Counter::$CYCLE === true) {
    Counter::$CYCLE = Counter::$CYCLE && $RAN_LOOP;
    break_trace();
  }
  Counter::$STATE = 2;
  Counter::$CYCLE = true;
  return;
}

<<__NEVER_INLINE>>
function outer() :mixed{ while_loop(); }

<<__NEVER_INLINE>>
function go() :mixed{
  try { outer(); } catch (Exception $e) { var_dump("CATCH"); }
}

<<__EntryPoint>>
function main() :mixed{ go(); go(); go(); go(); go(); go(); go(); go(); go(); go(); }
