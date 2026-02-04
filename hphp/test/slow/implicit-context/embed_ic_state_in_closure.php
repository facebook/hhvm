<?hh


class MemoSensitiveData implements HH\IPureMemoizeParam {
  public function getPayload()[]: int {
    return 42;
  }
  public function getInstanceKey()[]: string {
    return 'strkey';
  }
}

abstract final class IntContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = MemoSensitiveData;
  const ctx CRun = [zoned];
  public static function set<T>(MemoSensitiveData $context, (function (): T) $f)[zoned, ctx $f]: T {
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: this::TData {
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

function get_closure_from_value_state(): (function (): void) {
  return IntContext::set(
    new MemoSensitiveData(),
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_closure(
      () ==> {
        echo "Calling closure with IC value: ".(string)IntContext::getContext()->getPayload()."\n";
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

function get_async_closure_from_value_state(): (function (): Awaitable<void>) {
  return IntContext::set(
    new MemoSensitiveData(),
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_async_closure(
      async () ==> {
        echo "Calling closure with IC value: ".(string)IntContext::getContext()->getPayload()."\n";
      },
    ),
  );
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  get_closure_from_backdoor()();
  get_closure_from_value_state()();

  await get_async_closure_from_backdoor()();
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
