<?hh

final class JoinDuringWakeup {
  public static ?Awaitable<mixed> $awaitable = null;

  public function __wakeup(): void {
    $awaitable = self::$awaitable;
    if ($awaitable === null) {
      throw new Exception('External thread event was not captured');
    }
    HH\Asio\join($awaitable);
  }
}

<<__DynamicallyCallable>>
function make_result(): JoinDuringWakeup {
  return new JoinDuringWakeup();
}

<<__EntryPoint>>
function main(): void {
  if (HH\execution_context() === 'xbox') {
    return;
  }

  ExternalThreadEventWaitHandle::setOnCreateCallback($awaitable ==> {
    JoinDuringWakeup::$awaitable = $awaitable;
  });

  try {
    HH\Asio\join(fb_gen_user_func_array(__FILE__, 'make_result', vec[]));
    echo "unexpected success\n";
  } catch (InvalidOperationException $exception) {
    echo $exception->getMessage(), "\n";
  }
}
