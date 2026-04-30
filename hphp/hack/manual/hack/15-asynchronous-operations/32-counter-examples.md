# Counter Examples

Here are some examples of common pitfalls when using async functions.

## Joining

In a properly structured code you should never need this. You should always refactor the code to be async.

But if you fail, it is possible to get the result of an awaitable in a non-async function using `join`:

```hack
async function join_async(): Awaitable<string> {
  return "Hello";
}

// In an async function, you would await an awaitable.
// In a non-async function, or the global scope, you can
// use `join` to force the the awaitable to run to its completion.

<<__EntryPoint>>
function main(): void {
  $s = \HH\Asio\join(join_async());
  \var_dump($s);
}
```

## Batching

You should not do this unless you are talking to a backend with poorly designed connection and session management.

Use rescheduling (via `HH\Asio\later`) to batch up operations to send multiple keys in a single request to a backend that has extremely high per request overhead:

```hack
async function b_one(string $key): Awaitable<string> {
  $subkey = await Batcher::lookup($key);
  return await Batcher::lookup($subkey);
}

async function b_two(string $key): Awaitable<string> {
  return await Batcher::lookup($key);
}

<<__EntryPoint>>
async function batching(): Awaitable<void> {
  concurrent {
    $one = await b_one('hello');
    $two = await b_two('world');
  }
  \printf("%s\n%s\n", $one, $two);
}

class Batcher {
  private static vec<string> $pendingKeys = vec[];
  private static ?Awaitable<dict<string, string>> $aw = null;

  public static async function lookup(string $key): Awaitable<string> {
    // Add this key to the pending batch
    self::$pendingKeys[] = $key;
    // If there's no awaitable about to start, create a new one
    if (self::$aw === null) {
      self::$aw = self::go();
    }
    // Wait for the batch to complete, and get our result from it
    $results = await self::$aw;
    return $results[$key];
  }

  private static async function go(): Awaitable<dict<string, string>> {
    // Let other awaitables get into this batch
    await \HH\Asio\later();
    // Now this batch has started; clear the shared state
    $keys = self::$pendingKeys;
    self::$pendingKeys = vec[];
    self::$aw = null;
    // Do the multi-key roundtrip
    return await multi_key_lookup($keys);
  }
}

async function multi_key_lookup(
  vec<string> $keys,
): Awaitable<dict<string, string>> {

  // lookup multiple keys, but, for now, return something random
  $r = dict[];
  foreach ($keys as $key) {
    $r[$key] = \str_shuffle("ABCDEF");
  }
  return $r;
}
```

## Polling

Yet another counter example. If a service does not have an async implementation, people invent various forms of polling, rather than doing
the right thing and implementing async API. This one shows an example of using rescheduling in a polling loop, which will result in CPU-busy
loop if ran concurrently with other I/O operations:

```hack
// Of course, this is all made up :)
class Polling {
  private int $count = 0;
  public function isReady(): bool {
    $this->count++;
    if ($this->count > 10) {
      return true;
    }
    return false;
  }
  public function getResult(): int {
    return 23;
  }
}

async function do_polling(Polling $p): Awaitable<int> {
  echo "do polling 1".\PHP_EOL;
  // No async function in Polling, so loop until we are ready, but let
  // other awaitables go via later()
  while (!$p->isReady()) {
    await \HH\Asio\later();
  }
  echo "\ndo polling 2".\PHP_EOL;
  return $p->getResult();
}

async function no_polling(): Awaitable<string> {
  echo '.';
  return \str_shuffle("ABCDEFGH");
}

<<__EntryPoint>>
async function polling_example(): Awaitable<void> {
  concurrent {
    await do_polling(new Polling());
    // To make this semi-realistic, call no_polling a bunch of times to show
    // that do_polling is waiting.
    await Vec\map_async(Vec\range(0, 49), async $_ ==> await no_polling());
    // If you uncomment this, do_polling() will busy loop and burn 1s of CPU time.
    // await \HH\Asio\usleep(1000000);
  }
}
```
