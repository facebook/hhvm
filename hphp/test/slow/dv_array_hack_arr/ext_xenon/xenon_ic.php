<?hh

// Basic Xenon test.  PHP stacks but no Async stacks.

abstract final class IntContext extends HH\HHVMTestMemoAgnosticImplicitContext {
  const type TData = int;
  const ctx CRun = [zoned];
  public static function setAsync<T>(int $context, (function (): T) $f)[zoned, ctx $f]: T {
    return parent::runWithAsync($context, $f);
  }
}

function has_ic(): void {
  usleep(100000);
}

function has_no_ic(): void {
  usleep(100000);
}

async function main(): Awaitable<void> {
  IntContext::setAsync(42, async () ==> {
    has_ic();
  });
  has_no_ic();
}
<<__EntryPoint>>
function entrypoint_xenon_init(): void {
  \HH\Asio\join(main());

  $stacks = xenon_get_data();

  $found_has_ic = false;
  $found_has_no_ic = false;

  $find_frame = ($stack, $name) ==>
    HH\Lib\C\any(
      $stack['stack'],
      $f ==> ($f['function'] ?? null) === $name,
    );
  foreach ($stacks as $stack) {
    if ($find_frame($stack, 'has_ic')) {
      $found_has_ic = true;
      if ($stack['implicitContext'] !== dict['IntContext' => 42]) {
        echo "Expected IntContext to be set\n";
      }
    }
    if ($find_frame($stack, 'has_no_ic')) {
      $found_has_no_ic = true;
      if ($stack['implicitContext'] !== dict[]) {
        echo "Expected IntContext to be not set\n";
      }
    }
  }

  if ($found_has_ic) {
    echo "Found has_ic\n";
  } else {
    echo "Did not find has_ic\n";
  }

  if ($found_has_no_ic) {
    echo "Found has_no_ic\n";
  } else {
    echo "Did not find has_no_ic\n";
  }
}
