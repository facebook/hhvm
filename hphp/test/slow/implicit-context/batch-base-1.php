<?hh

class MemoSensitiveData implements HH\IPureMemoizeParam {
  public function __construct(private string $p)[] {}
  public function getInstanceKey()[]: string {
    return $this->p;
  }
}

abstract class StringMemoContextBase extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = MemoSensitiveData;
  const ctx CRun = [defaults];

  final public static function prepare_(
    string $context,
  )[]: \HH\ImplicitContext\PreparedContext {
    return self::prepare(new MemoSensitiveData($context));
  }

  final public static function get(
  ): string {
    return parent::get()?->getInstanceKey() ?? '__NONE__';
  }
}

class StringAgnosticContext extends HH\HHVMTestMemoAgnosticImplicitContext {
  const type TData = string;
  const ctx CRun = [defaults];

  final public static function prepare_(
    this::TData $context,
  )[]: \HH\ImplicitContext\PreparedContext {
    return self::prepare($context);
  }

  final public static function get(
  ): string {
    return parent::get() ?? '__NONE__';
  }
}

abstract final class StringMemoContext1 extends StringMemoContextBase {}
abstract final class StringMemoContext2 extends StringMemoContextBase {}

class MemoTester {
  private static int $counter = 0;
  <<__Memoize(#KeyedByIC)>>
  static function get(): int {
    self::$counter++;
    return self::$counter;
  }
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $print_fn = () ==> {
    print HH\Lib\Str\format(
      "m1:%s m2:%s i:%s memo_counter:%d\n",
      StringMemoContext1::get(),
      StringMemoContext2::get(),
      StringAgnosticContext::get(),
      MemoTester::get(),
    );
  };
  $print_fn();

  // Sync
  HH\ImplicitContext\PreparedContext::runBatch(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext2::prepare_('b'),
    ],
    $print_fn,
  );
  HH\ImplicitContext\PreparedContext::runBatch(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext1::prepare_('b'),
    ],
    $print_fn,
  );
  HH\ImplicitContext\PreparedContext::runBatch(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext1::prepare_('b'),
      StringAgnosticContext::prepare_('c'),
    ],
    $print_fn,
  );

  // Async
  await HH\ImplicitContext\PreparedContext::runBatchAsync(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext2::prepare_('b'),
    ],
    async () ==> $print_fn(),
  );
  await HH\ImplicitContext\PreparedContext::runBatchAsync(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext1::prepare_('b'),
    ],
    async () ==> $print_fn(),
  );
  await HH\ImplicitContext\PreparedContext::runBatchAsync(
    vec[
      StringMemoContext1::prepare_('a'),
      StringMemoContext1::prepare_('b'),
      StringAgnosticContext::prepare_('c'),
    ],
    async () ==> $print_fn(),
  );
}
