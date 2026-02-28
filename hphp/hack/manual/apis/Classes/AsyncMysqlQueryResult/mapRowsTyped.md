
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the actual rows returned by the successful query, each row
including the name and typed-value for each column




``` Hack
public function mapRowsTyped(): Vector<Map<string, mixed>>;
```




The rows are returned as a [` Vector `](/apis/Classes/HH/Vector/) of [` Map `](/apis/Classes/HH/Map/) objects. The [` Map `](/apis/Classes/HH/Map/) objects map
column names to (possibly `` null ``) ``` mixed ``` values (e.g., an ```` INTEGER ```` column
will come back as an ````` int `````.)




## Returns




+ [` Vector<Map<string, `](/apis/Classes/HH/Vector/)`` mixed>> `` - A [` Vector `](/apis/Classes/HH/Vector/) of [` Map `](/apis/Classes/HH/Map/) objects, where the [` Vector `](/apis/Classes/HH/Vector/) elements are the
  rows and the [` Map `](/apis/Classes/HH/Map/) elements are the column names and typed values
  associated with that row.




## Examples




When executing a query, you can get the rows returned from it in the form of a [` Vector `](/apis/Classes/HH/Vector/) of [` Map `](/apis/Classes/HH/Map/) objects, where each key of the [` Map `](/apis/Classes/HH/Map/) is a column name. This example shows how to use [` AsyncMysqlQueryResult::mapRowsTyped `](/apis/Classes/AsyncMysqlQueryResult/mapRowsTyped/) to get that [` Map `](/apis/Classes/HH/Map/). A resulting [` Map `](/apis/Classes/HH/Map/) may look like:




```
object(HH\Vector)#9 (2) {
  [0]=>
  object(HH\Map)#10 (2) {
    ["name"]=>
    string(11) "Joel Marcey"
    ["age"]=>
    int(41)
  }
  [1]=>
  object(HH\Map)#11 (2) {
    ["name"]=>
    string(11) "Fred Emmott"
    ["age"]=>
    int(26)
  }
}
```




Note that all values in the [` Map `](/apis/Classes/HH/Map/) returned from `` mapRowsTyped `` will be the actual typed representation of the database type, or ``` null ```. Above you can see we have ```` string ```` and ````` int `````. If you want just `````` string `````` values for everything, use [` mapRows `](/apis/Classes/AsyncMysqlQueryResult/mapRows/)




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
  $result = await $conn->query(
    'SELECT name, age FROM test_table WHERE userID < 50',
  );
  \var_dump($result->mapRowsTyped()->count() === $result->numRows());
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
