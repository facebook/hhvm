<?hh

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function set<T>(int $context, (function (): T) $f)[zoned, ctx $f]: T {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

<<__Memoize>>
function memo(): void {
}

function get_closure_from_backdoor(): (function (): void) {
  return HH\Coeffects\backdoor(
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_closure(
      () ==> { memo(); echo "(no warning expected)"; },
    ),
  );
}

<<__Memoize(#SoftMakeICInaccessible)>>
function get_closure_from_soft_mici(): (function (): void) {
  return HH\ImplicitContext\embed_implicit_context_state_in_closure(
    () ==> memo(),
  );
}

<<__Memoize(#MakeICInaccessible)>>
function get_closure_from_mici(): (function (): void) {
  return HH\ImplicitContext\embed_implicit_context_state_in_closure(
    () ==> memo(),
  );
}

function get_closure_from_soft_set(): (function (): void) {
  return HH\ImplicitContext\soft_run_with(
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_closure(
      () ==> memo(),
    ),
    'test key',
  );
}

function get_closure_from_value_state(): (function (): void) {
  return IntContext::set(
    42,
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_closure(
      () ==> {
        echo "Calling closure with IC value: ".(string)IntContext::getContext()."\n";
      },
    ),
  );
}

function get_async_closure_from_backdoor(): (function (): Awaitable<void>) {
  return HH\Coeffects\backdoor(
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
      async () ==> { memo(); echo "(no warning expected)"; },
    ),
  );
}

<<__Memoize(#SoftMakeICInaccessible)>>
function get_async_closure_from_soft_mici(): (function (): Awaitable<void>) {
  return HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
    async () ==> { memo(); },
  );
}

<<__Memoize(#MakeICInaccessible)>>
function get_async_closure_from_mici(): (function (): Awaitable<void>) {
  return HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
    async () ==> { memo(); },
  );
}

function get_async_closure_from_soft_set(): (function (): Awaitable<void>) {
  return HH\ImplicitContext\soft_run_with(
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
      async () ==> { memo(); },
    ),
    'test key',
  );
}

function get_async_closure_from_value_state(): (function (): Awaitable<void>) {
  return IntContext::set(
    42,
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
      async () ==> {
        echo "Calling closure with IC value: ".(string)IntContext::getContext()."\n";
      },
    ),
  );
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  HH\ImplicitContext\soft_run_with(
    () ==> get_closure_from_backdoor()(),
    'This one happens via soft_run_with to distinguish null state',
  );
  get_closure_from_soft_mici()();
  try {
    get_closure_from_mici()();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
  get_closure_from_soft_set()();
  get_closure_from_value_state()();

  await HH\ImplicitContext\soft_run_with_async(
    async () ==> await get_closure_from_backdoor()(),
    'This one happens via soft_run_with to distinguish null state',
  );
  await get_async_closure_from_soft_mici()();
  try {
    await get_async_closure_from_mici()();
  } catch (Exception $e) {
    echo $e->getMessage();
  }
  await get_async_closure_from_soft_set()();
  await get_async_closure_from_value_state()();

  $cls_returns = HH\ImplicitContext\embed_implicit_context_state_in_closure(
    () ==> "returned value not lost",
  );
  var_dump($cls_returns());
  $cls_returns = HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
    async () ==> "returned value not lost",
  );
  var_dump(await $cls_returns());
}
