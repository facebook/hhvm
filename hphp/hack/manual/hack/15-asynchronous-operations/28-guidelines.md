# Guidelines

It might be tempting to just sprinkle `async`, `await` and `Awaitable` on all code. And while it is OK to have more `async` functions than
not&mdash;in fact, we should generally *not be afraid* to make a function `async` since there is no performance penalty for doing so&mdash;there are
some guidelines to follow in order to make the most efficient use of `async`.

## Be Liberal, but Careful, with Async

Should code be async or not?  It's okay to start with the answer *Yes* and then find a reason to say *No*.  For example, a simple "hello world"
program can be made async with no performance penalty.  And while there likely won't be any gain, there won't be any loss, either; the code is
simply ready for any future changes that may require async.

These two programs are, for all intents and purposes, equivalent.

```hack
function get_hello(): string {
  return "Hello";
}

<<__EntryPoint>>
function run_na_hello(): void {
  \var_dump(get_hello());
}
```

```hack
async function get_hello(): Awaitable<string> {
  return "Hello";
}

<<__EntryPoint>>
async function run_a_hello(): Awaitable<void> {
  $x = await get_hello();
  \var_dump($x);
}
```

Just make sure you are following the rest of the guidelines. Async is great, but you still have to consider things like caching, batching
and efficiency.

## Use Async Extensions

For the common cases where async would provide maximum benefit, HHVM provides convenient extension libraries to help make writing code
much easier. Depending on the use case scenario, we should liberally use:
* [MySQL](/hack/asynchronous-operations/extensions#mysql) for database access and queries.
* [cURL](/hack/asynchronous-operations/extensions#curl) for web page data and transfer.
* [McRouter](/hack/asynchronous-operations/extensions#mcrouter) for memcached-based operations.
* [Streams](/hack/asynchronous-operations/extensions#streams) for stream-based resource operations.

## Do Not Use Async in Loops

If you only remember one rule, remember this:

:::caution

Don't `await` in a loop.

:::

It totally defeats the purpose of async.

```hack
class User {
  public string $name;

  protected function __construct(string $name) {
    $this->name = $name;
  }

  public static function get_name(int $id): User {
    return new User(\str_shuffle("ABCDEFGHIJ").\strval($id));
  }
}

async function load_user(int $id): Awaitable<User> {
  // Load user from somewhere (e.g., database).
  // Fake it for now
  return User::get_name($id);
}

async function load_users_await_loop(vec<int> $ids): Awaitable<vec<User>> {
  $result = vec[];
  foreach ($ids as $id) {
    $result[] = await load_user($id);
  }
  return $result;
}

<<__EntryPoint>>
function runMe(): void {
  $ids = vec[1, 2, 5, 99, 332];
  $result = \HH\Asio\join(load_users_await_loop($ids));
  \var_dump($result[4]->name);
}
```

In the above example, the loop is doing two things:
1. Making the loop iterations the limiting factor on how this code is going to run. By the loop, we are guaranteed to get the users sequentially.
2. We are creating false dependencies. Loading one user is not dependent on loading another user.

Instead, we will want to use our async-aware mapping function, `Vec\map_async`.

```hack
class User {
  public string $name;

  protected function __construct(string $name) {
    $this->name = $name;
  }

  public static function get_name(int $id): User {
    return new User(\str_shuffle("ABCDEFGHIJ").\strval($id));
  }
}

async function load_user(int $id): Awaitable<User> {
  // Load user from somewhere (e.g., database).
  // Fake it for now
  return User::get_name($id);
}

async function load_users_no_loop(vec<int> $ids): Awaitable<vec<User>> {
  return await Vec\map_async(
    $ids,
    async $id ==> await load_user($id),
  );
}

<<__EntryPoint>>
function runMe(): void {
  $ids = vec[1, 2, 5, 99, 332];
  $result = \HH\Asio\join(load_users_no_loop($ids));
  \var_dump($result[4]->name);
}
```

## Considering Data Dependencies Is Important

Possibly the most important aspect in learning how to structure async code is understanding data dependency patterns. Here is the general
flow of how to make sure async code is data dependency correct:
1. Put each sequence of dependencies with no branching (chain) into its own `async` function.
2. Put each bundle of parallel chains into its own `async` function.
3. Repeat to see if there are further reductions.

Let's say we are getting blog posts of an author. This would involve the following steps:
1. Get the post ids for an author.
2. Get the post text for each post id.
3. Get comment count for each post id.
4. Generate final page of information

```hack
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
    list($post_data, $comment_count) = await Vec\from_async(vec[
      fetch_post_data($post_id),
      fetch_comment_count($post_id),
    ]);
    invariant($post_data is PostData, "This is good");
    invariant($comment_count is int, "This is good");
    return tuple($post_data, $comment_count);
  };

  // Transform the array of post IDs into an vec of results,
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

The above example follows our flow:
1. One function for each fetch operation (post ids, post text, comment count).
2. One function for the bundle of data operations (post text and comment count).
3. One top function that coordinates everything.

## Consider Batching

Wait handles can be rescheduled. This means that they can be sent back to the queue of the scheduler, waiting until other awaitables have
run. Batching can be a good use of rescheduling. For example, say we have high latency lookup of data, but we can send multiple keys for
the lookup in a single request.

```hack
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
  HH\Asio\join(batching());
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

In the example above, we reduce the number of roundtrips to the server containing the data information to two by batching the first lookup
in `b_one` and the lookup in `b_two`. The `Batcher::lookup` method helps enable this reduction. The call to `await HH\Asio\later` in
`Batcher::go` allows `Batcher::go` to be deferred until other pending awaitables have run.

So, `await Vec\from_async(vec[b_one('hello'), b_two('world')]);` has two pending awaitables. If `b_one` is called first, it calls `Batcher::lookup`, which
calls `Batcher::go`, which reschedules via `later`. Then HHVM looks for other pending awaitables. `b_two` is also pending. It calls
`Batcher::lookup` and then it gets suspended via `await self::$aw` because `Batcher::$aw` is no longer `null`. Now `Batcher::go)` resumes,
fetches, and returns the result.

## Don't Forget to Await an Awaitable

What do you think happens here?

```hack
async function speak(): Awaitable<void> {
  echo "one";
  await \HH\Asio\later();
  echo "two";
  echo "three";
}

<<__EntryPoint>>
async function forget_await(): Awaitable<void> {
  $handle = speak(); // This just gets you the handle
}
```

The answer is, the behavior is undefined. We might get all three echoes; we might only get the first echo; we might get nothing at all. The
only way to guarantee that `speak` runs to completion is to `await` it. `await` is the trigger to the async scheduler that allows HHVM to
appropriately suspend and resume `speak`; otherwise, the async scheduler will provide no guarantees with respect to `speak`.

## Minimize Undesired Side Effects

In order to minimize any unwanted side effects (e.g., ordering disparities), the creation and awaiting of awaitables should happen as close
together as possible.

```hack
async function get_curl_data(string $url): Awaitable<string> {
  return await \HH\Asio\curl_exec($url);
}

function possible_side_effects(): int {
  \sleep(1);
  echo "Output buffer stuff";
  return 4;
}

async function proximity(): Awaitable<void> {
  $handle = get_curl_data("http://example.com");
  possible_side_effects();
  await $handle; // instead you should await get_curl_data("....") here
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(proximity());
}
```

In the above example, `possible_side_effects` could cause some undesired behavior when we get to the point of awaiting the handle associated
with getting the data from the website.

Basically, don't depend on the order of output between runs of the same code; i.e., don't write async code where ordering is important. Instead
use dependencies via awaitables and `await`.

## Memoization May be Good, But Only Awaitables

Given that async is commonly used in operations that are time-consuming, memoizing (i.e., caching) the result of an async call can definitely be worthwhile.

The [`<<__Memoize>>`](/hack/attributes/predefined-attributes#__memoize) attribute does the right thing, so, use that. However, if to get
explicit control of the memoization, *memoize the awaitable* and not the result of awaiting it.

```hack
abstract final class MemoizeResult {
  private static async function time_consuming(): Awaitable<string> {
    await \HH\Asio\usleep(5000000);
    return "This really is not time consuming, but the sleep fakes it.";
  }

  private static ?string $result = null;

  public static async function memoize_result(): Awaitable<string> {
    if (self::$result === null) {
      self::$result =
        await self::time_consuming(); // don't memoize the resulting data
    }
    return self::$result;
  }
}
<<__EntryPoint>>
function runMe(): void {
  $t1 = \microtime(true);
  \HH\Asio\join(MemoizeResult::memoize_result());
  $t2 = \microtime(true) - $t1;
  $t3 = \microtime(true);
  \HH\Asio\join(MemoizeResult::memoize_result());
  $t4 = \microtime(true) - $t3;
  \var_dump($t4 < $t2); // The memoized result will get here a lot faster
}
```

On the surface, this seems reasonable. We want to cache the actual data associated with the awaitable. However, this can cause an undesired
race condition. Imagine that there are two other async functions awaiting the result of `memoize_result`, call them `A` and `B`.  The following
sequence of events can happen:
1. `A` gets to run, and `await`s `memoize_result`.
2. `memoize_result` finds that the memoization cache is empty (`$result` is `null`), so it `await`s `time_consuming`. It gets suspended.
3. `B` gets to run, and `await`s `memoize_result`. Note that this is a new awaitable; it's not the same awaitable as in 1.
4. `memoize_result` again finds that the memoization cache is empty, so it awaits `time_consuming` again. Now the time-consuming operation will
be done twice.

If `time_consuming` has side effects (e.g., a database write), then this could end up being a serious bug. Even if there are no side effects, it's
still a bug; the time-consuming operation is being done multiple times when it only needs to be done once.

Instead, memoize the awaitable:

```hack
abstract final class MemoizeAwaitable {
  private static async function time_consuming(): Awaitable<string> {
    await \HH\Asio\usleep(5000000);
    return "Not really time consuming but sleep."; // For type-checking purposes
  }

  private static ?Awaitable<string> $handle = null;

  public static function memoize_handle(): Awaitable<string> {
    if (self::$handle === null) {
      self::$handle = self::time_consuming(); // memoize the awaitable
    }
    return self::$handle;
  }
}

<<__EntryPoint>>
function runMe(): void {
  $t1 = \microtime(true);
  \HH\Asio\join(MemoizeAwaitable::memoize_handle());
  $t2 = \microtime(true) - $t1;
  $t3 = \microtime(true);
  \HH\Asio\join(MemoizeAwaitable::memoize_handle());
  $t4 = \microtime(true) - $t3;
  \var_dump($t4 < $t2); // The memoized result will get here a lot faster
}
```

This simply caches the handle and returns it verbatim; [Async Vs Awaitable](/hack/asynchronous-operations/introduction) explains this in more detail.

This would also work if it were an async function that awaited the handle after caching. This may seem unintuitive, because the function
`await`s every time it's executed, even on the cache-hit path. But that's fine: on every execution except the first, `$handle` is not `null`, so
a new call to `time_consuming` will not be started. The result of the one existing instance will be shared.

Either approach works, but the non-async caching wrapper can be easier to reason about.

## Use Lambdas Where Possible

The use of lambdas can cut down on code verbosity that comes with writing full closure syntax. Lambdas are quite useful in conjunction
with the [async utility helpers](/hack/asynchronous-operations/utility-functions).  For example, look how the following three ways to accomplish the same thing can be
shortened using lambdas.

```hack
async function fourth_root(num $n): Awaitable<float> {
  return sqrt(sqrt((float)$n));
}

async function normal_call(): Awaitable<vec<float>> {
  $nums = vec[64, 81];
  return await Vec\map_async($nums, fourth_root<>);
}

async function closure_call(): Awaitable<vec<float>> {
  $nums = vec[64, 81];
  $froots = async function(num $n): Awaitable<float> {
    return sqrt(sqrt((float)$n));
  };
  return await Vec\map_async($nums, $froots);
}

async function lambda_call(): Awaitable<vec<float>> {
  $nums = vec[64, 81];
  return await Vec\map_async($nums, async $num ==> sqrt(sqrt((float)$num)));
}

async function use_lambdas(): Awaitable<void> {
  $nc = await normal_call();
  $cc = await closure_call();
  $lc = await lambda_call();
  \var_dump($nc);
  \var_dump($cc);
  \var_dump($lc);
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(use_lambdas());
}
```

## Integrating async and non-async functions

If you need to call an async function from a non-async function, the best approach is to refactor so that the caller is also async. Sometimes this might need refactoring an unmanageable number of recursive call sites, so an alternative is available - but best avoided:

Imagine we are making a call to an `async` function `join_async` from a non-async scope. If refactoring to an entirely async call stack is not possible, `HH\Asio\join()` can be used to resolve the awaitable:

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

**THIS IS A COUNTEREXAMPLE**: in real-world code, the entrypoint should be made async instead.

`HH\Asio\join()` is not just a blocking form of `await`: no other Hack code in the current request will be executed until the awaitable you pass to `join()` is completed, blocking the entire request until then.

## Remember Async Is NOT Multi-threading

Async functions are not running at the same time. They are CPU-sharing via changes in wait state in executing code (i.e., pre-emptive
multitasking). Async still lives in the single-threaded world of normal Hack!

## `await` Is Not a General Expression

To strike a balance between flexibility, latency, and performance, we require
that `await`s only appear in **unconditionally consumed expression positions**.
For more details, see [Await As An Expression](/hack/asynchronous-operations/await-as-an-expression).
