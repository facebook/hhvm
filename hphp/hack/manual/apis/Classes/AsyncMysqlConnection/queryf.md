
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Execute a query with placeholders and parameters




``` Hack
public function queryf(
  HH\FormatString<HH\SQLFormatter> $pattern,
  ...$args,
): Awaitable<AsyncMysqlQueryResult>;
```




This is probably the more common of the two query methods, given its
flexibility and automatic escaping in most string cases.




For example:
[` queryf("SELECT `](/apis/Classes/AsyncMysqlConnection/queryf/)`` %C FROM %T WHERE %C %=s", $col1, $table, $col2, $value); ``




The supported placeholders are:

+ ` %T `   table name
+ ` %C `   column name
+ ` %s `   nullable string (will be escaped)
+ ` %d `   integer
+ ` %f `   float
+ ` %=s `  nullable string comparison - expands to either:
  `` = 'escaped_string' ``
  ``` IS NULL ```
+ ` %=d `  nullable integer comparison
+ ` %=f `  nullable float comparison
+ ` %Q `   raw SQL query. The typechecker intentionally does not recognize
  this, however, you can use it in combination with // UNSAFE
  if absolutely required. Use this at your own risk as it could
  open you up for SQL injection.
+ ` %Lx `  where `` x `` is one of ``` T ```, ```` C ````, ````` s `````, `````` d ``````, or ``````` f ```````, represents a list
  of table names, column names, nullable strings, integers or
  floats, respectively. Pass a [` Vector `](/apis/Classes/HH/Vector/) of values to have it
  expanded into a comma-separated list. Parentheses are not
  added automatically around the placeholder in the query string,
  so be sure to add them if necessary.




With the exception of ` %Q `, any strings provided will be properly
escaped.




## Parameters




* ` HH\FormatString<HH\SQLFormatter> $pattern ` - The query string with any placeholders.
* ` ...$args ` - The real values for all of the placeholders in your query
  string. You must have as many values as you do
  placeholders.




## Returns




- [` Awaitable<AsyncMysqlQueryResult> `](/apis/Classes/HH/Awaitable/) - an [` Awaitable `](/apis/Classes/HH/Awaitable/) representing the result of your query. Use
  `` await `` or ``` join ``` to get the actual [` AsyncMysqlQueryResult `](/apis/Classes/AsyncMysqlQueryResult/)
  object.




## Examples




The following example shows how to use [` AsyncMysqlConnection::queryf `](/apis/Classes/AsyncMysqlConnection/queryf/). First you get a connection from an [` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/); then you decide what parameters you want to pass as query placeholders.




~~~ basic-usage.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}

async function get_data(
  \AsyncMysqlConnection $conn,
  string $col,
  int $id,
): Awaitable<\AsyncMysqlQueryResult> {
  return await $conn->queryf(
    'SELECT %C FROM test_table where userID = %d',
    $col,
    $id,
  );
}

async function simple_queryf(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await get_data($conn, 'name', 1);
  $x = $result->numRows();
  $result = await get_data($conn, 'name', 2);
  $conn->close();
  return $x + $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_queryf();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~




The following example uses the ` %=s ` placeholder in order to allow you to check whether an email address with the provided `` string `` exists in the table, or, if ``` null ``` is passed, whether there is a user with a ```` null ```` email address.




~~~ percent-equal-placeholders.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}

async function get_data(
  \AsyncMysqlConnection $conn,
  string $col,
  ?string $email,
): Awaitable<\AsyncMysqlQueryResult> {
  // %=s allows you to check an actual string value or IS NULL
  return await $conn->queryf(
    'SELECT %C FROM test_table where email %=s',
    $col,
    $email,
  );
}

async function simple_queryf(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await get_data($conn, 'name', 'joelm@fb.com');
  $x = $result->numRows();
  $result = await get_data($conn, 'name', null);
  $conn->close();
  return $x + $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_queryf();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(18)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~




The following example shows how to use the ` %L ` placeholder for [` AsyncMysqlConnection::queryf `](/apis/Classes/AsyncMysqlConnection/queryf/). First you get a connection from an [` AsyncMysqlConnectionPool `](/apis/Classes/AsyncMysqlConnectionPool/); then we are passing a vector of ids to used in the placeholder. The placeholder ends up being `` %Ld `` since the ids are integers.




~~~ percent-L-placeholders.hack
use \Hack\UserDocumentation\API\Examples\AsyncMysql\ConnectionInfo as CI;

async function connect(
  \AsyncMysqlConnectionPool $pool,
): Awaitable<\AsyncMysqlConnection> {
  return await $pool->connect(
    CI::$host,
    CI::$port,
    CI::$db,
    CI::$user,
    CI::$passwd,
  );
}

async function get_data(
  \AsyncMysqlConnection $conn,
  Vector<int> $ids,
): Awaitable<\AsyncMysqlQueryResult> {
  return await $conn->queryf(
    'SELECT name FROM test_table where userID IN (%Ld)',
    $ids,
  );
}

async function percent_L_queryf(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $ids = Vector {1, 2};
  $result = await get_data($conn, $ids);
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await percent_L_queryf();
  \var_dump($r);
}
```.hhvm.expectf
int(%d)
```.example.hhvm.out
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
