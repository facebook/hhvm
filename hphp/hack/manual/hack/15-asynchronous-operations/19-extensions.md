# Extensions

Async in and of itself is a highly useful construct that can provide possible time-saving through its cooperative multitasking
infrastructure. Async is especially useful with database access and caching, web resource access, and dealing with streams.

## MySQL

The async MySQL extension provides a Hack API to query MySQL and similar databases. All relevant methods return an `Awaitable`,
which gives your code control over how it spends the time until the result is ready.

The [full API](/apis/Classes/AsyncMysqlConnection/) contains all of the classes and methods available for accessing MySQL via async; we
will only cover a few of the more common scenarios here.

The primary class for connecting to a MySQL database is [`AsyncMysqlConnectionPool`](/apis/Classes/AsyncMysqlClient/) and its
primary method is the `async` [`connect`](/apis/Classes/AsyncMysqlClient/connect/).

The primary class for querying a database is [`AsyncMysqlConnection`](/apis/Classes/AsyncMysqlConnection/) with the three main query methods:
`AsyncMysqlConnection::query`, `AsyncMysqlConnection::queryf`, and `AsyncMysqlConnection::queryAsync`
all of which are `async`. The naming conventions for async methods have not been applied consistently.
There is also a method which turns a raw string into an SQL escaped string `AsyncMysqlConnection::escapeString`.
This method should only be used in combination with `AsyncMysqlConnection::query`.

## Choosing a query method

### `AsyncMysqlConnection::queryAsync`
This method is the go-to for safe SQL queries. It handles escaping user input automatically and the `%Q` placeholder can be used to insert query fragments into other query fragments.

For example:
 - selecting a post by ID
```SQL
SELECT title, body FROM posts WHERE id %=d
```
 - Advanced search, which may allow the user to specify a handful of parameters
```SQL
SELECT %LC FROM %T WHERE %Q
```
 - Inserting _n_ rows in one big INSERT INTO statement
```SQL
INSERT INTO %T (%LC) VALUES (%Q)
```

For a list of all placeholders supported by `queryAsync`, see `HH\Lib\SQL\QueryFormatString`.
Most placeholders are documented with examples here `AsyncMysqlConnection::queryf`.
The types for the `%Lx` placeholders is not a Hack Collection when using queryAsync, but a Hack array instead.
The documentation for `%Q` is flat out wrong for using `queryAsync` and should be ignored.

### `AsyncMysqlConnection::query`
This method is **dangerous** when used wrong. It accepts a _raw string_ and passes it to the database without scanning for dangerous characters or doing any automatic escaping.
If your query contains ANY unescaped input, you are putting your database at risk.

Using this method is preferred for one use case:

When you can type out the query outright, without string interpolation or string concatenation.
You can pass in a _hardcoded_ query. This can be preferred over `queryAsync` and `queryf`, since you can write your SQL string literals without having to write a `%s` placeholder. Which makes it easier to change the query later without the risk of messing up which parameter goes with which `%s`.

This method can also be used to create queries by doing string manipulation. If you are doing this, you, the developer, must take responsibility for sanitizing the data. Escape everything that needs to be escaped and make triple sure there is not a sneaky way to get raw user data into the concatenated string at any point. As said, this method is dangerous and this is why.

### `AsyncMysqlConnection::queryf`
Is an older API which does what `queryAsync` does, but with more restrictions. It uses Hack Collections instead of Hack arrays for its `%Lx` arguments. There is no way to create fragments of queries for safe query building. It is also not possible to build a query without having an `\AsyncMysqlConnection`. New code should use `queryAsync` instead.

## Getting results
The primary class for retrieving results from a query is an abstract class called `AsyncMysqlResult`, which itself has two concrete
subclasses called [`AsyncMysqlQueryResult`](/apis/Classes/AsyncMysqlQueryResult/) and
[`AsyncMysqlErrorResult`](/apis/Classes/AsyncMysqlErrorResult/). The main methods on these classes are
[`vectorRows`](/apis/Classes/AsyncMysqlQueryResult/vectorRows/) | [`vectorRowsTyped`](/apis/Classes/AsyncMysqlQueryResult/vectorRowsTyped/) and [`mapRows`](/apis/Classes/AsyncMysqlQueryResult/mapRows/) | [`mapRowsTyped`](/apis/Classes/AsyncMysqlQueryResult/mapRowsTyped/), which are *non-async*.

### When to use the `____Typed` variant over its untyped counterpart.

The typed functions will help you by turning database integers into Hack `int`s and _some, but not all_ fractional numbers into Hack `float`s.
This is almost always preferred, except for cases where the result set includes unsigned 64-bit integers (UNSIGNED BIGINT).
These numbers may be larger than the largest representable signed 64-bit integer and can therefore not be used in a Hack program.
The runtime currently returns the largest signed 64-bit integer for all values which exceed the maximum signed 64-bit integer.

The untyped function will return all fields as either a Hack `string` or a Hack `null`. So an integer `404` in the database would come back as `"404"`.

Here is a simple example that shows how to get a user name from a database using this extension:

```hack no-extract
async function get_connection(): Awaitable<AsyncMysqlConnection> {
  // Get a connection pool with default options
  $pool = new AsyncMysqlConnectionPool(darray[]);
  // Change credentials to something that works in order to test this code
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}

async function fetch_user_name(
  AsyncMysqlConnection $conn,
  int $user_id,
): Awaitable<?string> {
  // Your table and column may differ, of course
  $result = await $conn->queryf(
    'SELECT name from test_table WHERE userID = %d',
    $user_id,
  );
  // There shouldn't be more than one row returned for one user id
  invariant($result->numRows() === 1, 'one row exactly');
  // A vector of vector objects holding the string values of each column
  // in the query
  $vector = $result->vectorRows();
  return $vector[0][0]; // We had one column in our query
}

async function get_user_info(
  AsyncMysqlConnection $conn,
  string $user,
): Awaitable<Vector<Map<string, ?string>>> {
  $result = await $conn->queryf(
    'SELECT * from test_table WHERE name %=s',
    $user,
  );
  // A vector of map objects holding the string values of each column
  // in the query, and the keys being the column names
  $map = $result->mapRows();
  return $map;
}

<<__EntryPoint>>
async function async_mysql_tutorial(): Awaitable<void> {
  $conn = await get_connection();
  if ($conn !== null) {
    $result = await fetch_user_name($conn, 2);
    var_dump($result);
    $info = await get_user_info($conn, 'Fred Emmott');
    var_dump($info is vec<_>);
    var_dump($info[0] is dict<_, _>);
  }
}
```

### Connection Pools

The async MySQL extension does *not* support multiplexing; each concurrent query requires its own connection. However, the extension
does support connection pooling.

The async MySQL extension provides a mechanism to pool connection objects so we don't have to create a new connection every time we
want to make a query. The class is [`AsyncMysqlConnectionPool`](/apis/Classes/AsyncMysqlConnectionPool/) and one can be created like this:

```hack no-extract
function get_pool(): AsyncMysqlConnectionPool {
  return new AsyncMysqlConnectionPool(
    darray['pool_connection_limit' => 100],
  ); // See API for more pool options
}

async function get_connection(): Awaitable<AsyncMysqlConnection> {
  $pool = get_pool();
  $conn = await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
  return $conn;
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $conn = await get_connection();
  var_dump($conn);
}
```

It is ***highly recommended*** that connection pools are used for MySQL connections; if for some reason we really need one, single asynchronous
client, there is an [`AsyncMysqlClient`](/apis/Classes/AsyncMysqlClient/) class that provides a
[`connect`](/apis/Classes/AsyncMysqlClient/connect/) method.

## MCRouter

MCRouter is a memcached protocol-routing library. To help with  [memcached](http://php.net/manual/en/book.memcached.php) deployment, it
provides features such as connection pooling and prefix-based routing.

The async MCRouter extension is basically an async subset of the Memcached extension that is part of HHVM. The primary class is
`MCRouter`. There are two ways to create an instance of an MCRouter object. The
[`createSimple`](/apis/Classes/MCRouter/createSimple/) method takes a vector of server addresses where memcached is running. The
more configurable [`__construct`](/apis/Classes/MCRouter/__construct/) method allows for more option tweaking. After getting an object,
we can use the `async` versions of the core memcached protocol methods like [`add`](/apis/Classes/MCRouter/add/),
[`get`](/apis/Classes/MCRouter/get/) and [`del`](/apis/Classes/MCRouter/del/).

Here is a simple example showing how one might get a user name from memcached:

```hack no-extract
function get_mcrouter_object(): MCRouter {
  $servers = Vector {\getenv('HHVM_TEST_MCROUTER')};
  $mc = MCRouter::createSimple($servers);
  return $mc;
}

async function add_user_name(
  MCRouter $mcr,
  int $id,
  string $value,
): Awaitable<void> {
  $key = 'name:'.$id;
  await $mcr->set($key, $value);
}

async function get_user_name(\MCRouter $mcr, int $user_id): Awaitable<string> {
  $key = 'name:'.$user_id;
  try {
    $res = await HH\Asio\wrap($mcr->get($key));
    if ($res->isSucceeded()) {
      return $res->getResult();
    }
    return "";
  } catch (\MCRouterException $ex) {
    echo $ex->getKey().\PHP_EOL.$ex->getOp();
    return "";
  }
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $mcr = get_mcrouter_object();
  await add_user_name($mcr, 1, 'Joel');
  $name = await get_user_name($mcr, 1);
  var_dump($name); // Should print "Joel"
}
```

If an issue occurs when using this protocol, two possible exceptions can be thrown: `MCRouterException` when something goes wrong with
a core option, like deleting a key; `MCRouterOptionException` when the provide option list can't be parsed.

## cURL

Hack currently provides two async functions for [cURL](http://curl.haxx.se/).

### `curl_multi_await`

cURL provides a data transfer library for URLs. The async cURL extension provides two functions, one of which is a wrapper around the
other. `curl_multi_await` is the async version of HHVM's `curl_multi_select`. It waits until there is activity on the cURL handle and
when it completes, we use `curl_multi_exec` to process the result, just as we would in the non-async situation.

```hack no-extract
async function curl_multi_await(resource $mh, float $timeout = 1.0): Awaitable<int>;
```

### `curl_exec`

The function `HH\Asio\curl_exec` is a wrapper around `curl_multi_await`. It is easy to use as we don't necessarily have to worry about
resource creation since we can just pass a string URL to it.

```hack no-extract
namespace HH\Asio {
  async function curl_exec(mixed $urlOrHandle): Awaitable<string>;
}
```

Here is an example of getting a vector of URL contents, using a lambda expression to cut down on the code verbosity that would come with
full closure syntax:

```hack no-extract
function get_urls(): vec<string> {
  return vec[
    "http://example.com",
    "http://example.net",
    "http://example.org",
  ];
}

async function get_combined_contents(
  vec<string> $urls,
): Awaitable<vec<string>> {
  // Use lambda shorthand syntax here instead of full closure syntax
  $handles = Vec\map_with_key(
    $urls,
    ($idx, $url) ==> HH\Asio\curl_exec($url),
  );
  $contents = await Vec\from_async($handles);
  echo C\count($contents)."\n";
  return $contents;
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(get_combined_contents(get_urls()));
}
```

## Streams

The async stream extension has one function, [`stream_await`](/apis/Functions/stream_await/), which is functionally similar
to HHVM's [`stream_select`](http://php.net/manual/en/function.stream-select.php). It waits for a stream to enter a state (e.g.,
`STREAM_AWAIT_READY`), but without the multiplexing functionality of [`stream_select`](http://php.net/manual/en/function.stream-select.php). We
can use [HH\Lib\Vec\from_async](/hsl/Functions/HH.Lib.Vec/from_async/) to await multiple stream handles, but the resulting combined awaitable won't be complete
until all of the underlying streams have completed.

```hack no-extract
async function stream_await(resource $fp, int $events, float $timeout = 0.0): Awaitable<int>;
```

The following example shows how to use [`stream_await`](/apis/Functions/stream_await/) to write to resources:

```hack
function get_resources(): vec<resource> {
  $r1 = fopen('php://stdout', 'w');
  $r2 = fopen('php://stdout', 'w');
  $r3 = fopen('php://stdout', 'w');

  return vec[$r1, $r2, $r3];
}

async function write_all(vec<resource> $resources): Awaitable<void> {
  $write_single_resource = async function(resource $r) {
    $status = await stream_await($r, STREAM_AWAIT_WRITE, 1.0);
    if ($status === STREAM_AWAIT_READY) {
      fwrite($r, str_shuffle('ABCDEF').\PHP_EOL);
    }
  };
  // You will get 3 shuffled strings, each on a separate line.
  await Vec\from_async(\array_map($write_single_resource, $resources));
}

<<__EntryPoint>>
function main(): void {
  HH\Asio\join(write_all(get_resources()));
}
```
