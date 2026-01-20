
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the actual rows returned by the successful query, each row
including the values for each column




``` Hack
public function vectorRows(): Vector<KeyedContainer<int, ?string>>;
```




All values come back as ` string `s. If you want typed values, use
[` vectorRowsTyped() `](/apis/Classes/AsyncMysqlQueryResult/vectorRowsTyped/).




The rows are returned as a [` Vector `](/apis/Classes/HH/Vector/) of [` Vector `](/apis/Classes/HH/Vector/) objects which hold the
(possibly `` null ``) ``` string ``` values of each column in the order of the
original query.




## Returns




+ [` Vector<KeyedContainer<int, `](/apis/Classes/HH/Vector/)`` ?string>> `` - A [` Vector `](/apis/Classes/HH/Vector/) of [` Vector `](/apis/Classes/HH/Vector/) objects, where the outer [` Vector `](/apis/Classes/HH/Vector/)
  represents the rows and each inner [` Vector `](/apis/Classes/HH/Vector/) represent the
  column values for each row.




## Examples




When executing a query, you can get the rows returned from it in the form of a [` Vector `](/apis/Classes/HH/Vector/) of [` Vector `](/apis/Classes/HH/Vector/) objects, where each value of the [` Vector `](/apis/Classes/HH/Vector/) is a column value. This example shows how to use [` AsyncMysqlQueryResult::vectorRows `](/apis/Classes/AsyncMysqlQueryResult/vectorRows/) to get that [` Vector `](/apis/Classes/HH/Vector/). A resulting [` Vector `](/apis/Classes/HH/Vector/) may look like:




```
object(HH\Vector)#9 (2) {
  [0]=>
  object(HH\Vector)#10 (1) {
    [0]=>
    string(11) "Joel Marcey"
  }
  [1]=>
  object(HH\Vector)#11 (1) {
    [0]=>
    string(11) "Fred Emmott"
  }
}
```




Note that all values in the [` Vector `](/apis/Classes/HH/Vector/) returned from `` vectorRows `` will be ``` string ``` or ```` null ````. If you want specifically-typed values, use [` vectorRowsTyped `](/apis/Classes/AsyncMysqlQueryResult/vectorRowsTyped/).




Also understand that if you want the actual column names associated with the values in the [` Vector `](/apis/Classes/HH/Vector/), you should use [` mapRows `](/apis/Classes/AsyncMysqlQueryResult/mapRows/) instead.




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
async function simple_query(): Awaitable<int> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT name FROM test_table WHERE userID < 50');
  \var_dump($result->vectorRows()->count() === $result->numRows());
  $conn->close();
  return $result->numRows();
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
bool(true)
int(%d)
```.example.hhvm.out
bool(true)
int(1)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
