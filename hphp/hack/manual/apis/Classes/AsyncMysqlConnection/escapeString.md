
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Escape a string to be safe to include in a raw query




``` Hack
public function escapeString(
  string $data,
): string;
```




Use this method to ensure your query is safe from, for example, SQL
injection if you are not using an API that automatically escapes
queries.




We strongly recommend using [` queryf() `](/apis/Classes/AsyncMysqlConnection/queryf/) instead, which automatically
escapes string parameters.




This method is equivalent to PHP's
[mysql_real_escape_string()](<http://goo.gl/bnxqtE>).




## Parameters




+ ` string $data ` - The string to properly escape.




## Returns




* ` string ` - The escaped string.




## Examples




The following example shows you how to use [` AsyncMysqlConnection::escapeString `](/apis/Classes/AsyncMysqlConnection/escapeString/)
in order to make sure any string pass to something like
[` AsyncMysqlConnection::query `](/apis/Classes/AsyncMysqlConnection/query/) is safe for a database query. This is similar to
[` mysql_real_escape_string `](<http://php.net/manual/en/function.mysql-real-escape-string.php>).




We *strongly* recommend using an API like [` AsyncMysqlConnection::queryf `](/apis/Classes/AsyncMysqlConnection/queryf/) instead,
which automatically escapes strings passed to `` %s `` placeholders.




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
  string $name,
): Awaitable<\AsyncMysqlQueryResult> {
  /* DON'T DO THIS!
   *
   * Use AsyncMysqlConnection::queryf() instead, which automatically escapes
   * strings for %s placeholders.
   */
  $escaped_name = $conn->escapeString($name);
  \var_dump($escaped_name);
  return await $conn->query(
    "SELECT age FROM test_table where name = '".$escaped_name."'",
  );
}
async function simple_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await get_data($conn, 'Joel Marcey');
  $x = $result->numRows();
  $result = await get_data($conn, 'Daffy\nDuck');
  $conn->close();
  return $x + $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
}
```.hhvm.expect
string(11) "Joel Marcey"
string(12) "Daffy\\nDuck"
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
