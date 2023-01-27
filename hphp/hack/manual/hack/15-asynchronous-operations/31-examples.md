Here are some examples representing a slew of possible async scenarios. Obviously, this does not cover all possible situations, but they
provide an idea of how and where async can be used effectively. Some of these examples are found spread out through the rest of the async
documentation; they are added here again for consolidation purposes.

## Basic

This example shows the basic tenets of async, particularly the keywords used:

```Hack
// async specifies a function will return an awaitable. Awaitable<string> means
// that the awaitable will ultimately return a string when complete
async function trivial(): Awaitable<string> {
  return "Hello";
}

<<__EntryPoint>>
async function call_trivial(): Awaitable<void> {
  // These first two lines could be combined into
  //     $result = await trivial();
  // but wanted to show the process

  // get awaitable that you can wait for completion
  $aw = trivial();
  // wait for the operation to complete and get the result
  $result = await $aw;
  echo $result; // "Hello"
}
```

## Joining

To get the result of an awaitable in a non-async function, use `join`:

```Hack
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

## Async Closures and Lambdas

Closure and lambda expressions can involve async functions:

```Hack
<<__EntryPoint>>
async function closure_async(): Awaitable<void> {
  // closure
  $hello = async function(): Awaitable<string> {
    return 'Hello';
  };
  // lambda
  $bye = async ($str) ==> $str;

  // The call style to either closure or lambda is the same
  $rh = await $hello();
  $rb = await $bye("bye");

  echo $rh." ".$rb.\PHP_EOL;
}
```

## Data Fetching

This shows a way to organize async functions such that we have a nice clean data dependency graph:

```Hack
class PostData {
  // using constructor argument promotion
  public function __construct(public string $text) {}
}

async function fetch_all_post_ids_for_author(
  int $author_id,
): Awaitable<vec<int>> {

  // Query database, etc., but for now, just return made up stuff
  return vec[4, 53, 99];
}

async function fetch_post_data(int $post_id): Awaitable<PostData> {
  // Query database, etc. but for now, return something random
  return new PostData(\str_shuffle("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
}

async function fetch_comment_count(int $post_id): Awaitable<int> {
  // Query database, etc., but for now, return something random
  return \rand(0, 50);
}

async function fetch_page_data(
  int $author_id,
): Awaitable<vec<(PostData, int)>> {

  $all_post_ids = await fetch_all_post_ids_for_author($author_id);
  // An async closure that will turn a post ID into a tuple of
  // post data and comment count
  $post_fetcher = async function(int $post_id): Awaitable<(PostData, int)> {
    concurrent {
      $post_data = await fetch_post_data($post_id);
      $comment_count = await fetch_comment_count($post_id);
    }
    return tuple($post_data, $comment_count);
    // alternatively:
    $_return = tuple(
      await fetch_post_data($post_id),
      await fetch_comment_count($post_id),
    );
  };

  // Transform the array of post IDs into a vec of results,
  // using the Vec\map_async function
  return await Vec\map_async($all_post_ids, $post_fetcher);
}

async function generate_page(int $author_id): Awaitable<string> {
  $tuples = await fetch_page_data($author_id);
  $page = "";
  foreach ($tuples as $tuple) {
    list($post_data, $comment_count) = $tuple;
    // Normally render the data into HTML, but for now, just create a
    // normal string
    $page .= $post_data->text." ".$comment_count.\PHP_EOL;
  }
  return $page;
}

<<__EntryPoint>>
function main(): void {
  print \HH\Asio\join(generate_page(13324)); // just made up a user id
}
```

## Batching

Use rescheduling (via `HH\Asio\later`) to batch up operations to send multiple keys in a single request over a high latency network (for
example purposes, the network isn't high latency, but just returns something random):

```Hack
async function b_one(string $key): Awaitable<string> {
  $subkey = await Batcher::lookup($key);
  return await Batcher::lookup($subkey);
}

async function b_two(string $key): Awaitable<string> {
  return await Batcher::lookup($key);
}

async function batching(): Awaitable<void> {
  $results = await Vec\from_async(vec[b_one('hello'), b_two('world')]);
  \printf("%s\n%s\n", $results[0], $results[1]);
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(batching());
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

We can use rescheduling in a polling loop to allow other awaitables to run. A polling loop may be needed where a service does not have
an async function to add to the scheduler:

```Hack
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

async function polling_example(): Awaitable<void> {
  $handles = vec[do_polling(new Polling())];
  // To make this semi-realistic, call no_polling a bunch of times to show
  // that do_polling is waiting.
  for ($i = 0; $i < 50; $i++) {
    $handles[] = no_polling();
  }

  $results = await Vec\from_async($handles);
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(polling_example());
}
```
