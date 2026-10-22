<?hh

<<__NEVER_INLINE>>
async function jit_apply(
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  return await $callback();
}

<<__ALWAYS_INTERP, __NEVER_INLINE>>
async function interp_apply(
  (function(): Awaitable<int>) $callback,
): Awaitable<int> {
  return await $callback();
}

<<__ALWAYS_INTERP, __NEVER_INLINE>>
async function interp_eager_leaf(int $value): Awaitable<int> {
  return $value;
}

<<__NEVER_INLINE>>
async function jit_calls_interp_eager(int $value): Awaitable<int> {
  return await interp_eager_leaf($value);
}

<<__NEVER_INLINE>>
async function jit_callee_to_jit_caller(int $value): Awaitable<int> {
  return await jit_apply(async () ==> $value);
}

<<__ALWAYS_INTERP, __NEVER_INLINE>>
async function interp_callee_to_interp_caller(int $value): Awaitable<int> {
  return await interp_apply(async () ==> $value);
}

<<__NEVER_INLINE>>
async function interp_callee_to_jit_caller(int $value): Awaitable<int> {
  return await interp_apply(async () ==> $value);
}

<<__ALWAYS_INTERP, __NEVER_INLINE>>
async function jit_callee_to_interp_caller(int $value): Awaitable<int> {
  return await jit_apply(async () ==> $value);
}

function check_result(string $path, int $expected, int $actual): void {
  printf(
    "%s: %s 0x%x\n",
    $path,
    $actual === $expected ? 'ok' : 'bad',
    $actual,
  );
}

<<__EntryPoint>>
function main(): void {
  $jit_to_jit = __hhvm_intrinsics\launder_value(0x1122334455667788);
  check_result(
    'JIT -> JIT',
    $jit_to_jit,
    HH\Asio\join(jit_callee_to_jit_caller($jit_to_jit)),
  );

  $interp_to_interp = __hhvm_intrinsics\launder_value(0x2233445566778899);
  check_result(
    'Interpreter -> Interpreter',
    $interp_to_interp,
    HH\Asio\join(interp_callee_to_interp_caller($interp_to_interp)),
  );

  $interp_to_jit = __hhvm_intrinsics\launder_value(0x33445566778899aa);
  check_result(
    'Interpreter -> JIT',
    $interp_to_jit,
    HH\Asio\join(interp_callee_to_jit_caller($interp_to_jit)),
  );

  $jit_to_interp = __hhvm_intrinsics\launder_value(0x445566778899aabb);
  check_result(
    'JIT -> Interpreter',
    $jit_to_interp,
    HH\Asio\join(jit_callee_to_interp_caller($jit_to_interp)),
  );

  $interp_eager_to_jit = __hhvm_intrinsics\launder_value(0x5566778899aabbcc);
  check_result(
    'Eager-only Interpreter -> JIT',
    $interp_eager_to_jit,
    HH\Asio\join(jit_calls_interp_eager($interp_eager_to_jit)),
  );
}
