<?hh

<<__Memoize>>
function uncat_memoize(): void {
}

<<__Memoize(#SoftMakeICInaccessible)>>
async function soft_mici_function(): Awaitable<void> {
  uncat_memoize();
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  set_error_handler(
    ($_errno, $errstr, $_errfile, $_errline, $_errctx, $_errtrace, $ic_blame) ==> {
      echo $errstr."\n";
      var_dump($ic_blame);
      return true;
    },
  );

  HH\ImplicitContext\run_with_soft_inaccessible_state(
    uncat_memoize<>,
    'key1',
  );
  await HH\ImplicitContext\run_with_soft_inaccessible_state_async(
    async () ==> uncat_memoize(),
    'key2',
  );
  concurrent {
    await HH\ImplicitContext\run_with_soft_inaccessible_state_async(
      async () ==> {
        await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT, 1);
        uncat_memoize();
      },
      'key3',
    );
    await HH\ImplicitContext\run_with_soft_inaccessible_state_async(
      soft_mici_function<>,
      'key4',
    );
  }

  try {
    HH\ImplicitContext\soft_run_with(
      () ==> HH\ImplicitContext\run_with_soft_inaccessible_state(
        () ==> {},
        'key3',
      ),
      'key4',
    );
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
