
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get a certain field (column) value as a ` double `




``` Hack
public function getFieldAsDouble(
  mixed $field,
): float;
```




If the column from which you are retrieving the value is not an numeric
type, then an ` Exception ` is thrown.




## Parameters




+ ` mixed $field ` - the field index (`` int ``) or field name (``` string ```).




## Returns




* ` float ` - The `` double `` value of the field (column); or an ``` Exception ``` if it
  the column is not numeric.




## Examples




The following example shows how to get a field value as a ` float ` value via [` AsyncMysqlRow::getFieldAsDouble `](/apis/Classes/AsyncMysqlRow/getFieldAsDouble/). Assume this was the SQL used to create the example table:




```
CREATE TABLE test_table (
userID SMALLINT UNSIGNED ZEROFILL NOT NULL AUTO_INCREMENT,
name VARCHAR(40) NOT NULL,
age SMALLINT NULL,
email VARCHAR(60) NULL,
PRIMARY KEY (userID)
);
```




In this case, we are actually getting a field that is an ` int ` (or `` SMALLINT ``). But there is an automatic conversion to a ``` float ```.




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
async function simple_query(): Awaitable<float> {
  $pool = new \AsyncMysqlConnectionPool(darray[]);
  $conn = await connect($pool);
  $result = await $conn->query('SELECT * FROM test_table WHERE userID < 50');
  $conn->close();
  // A call to $result->rowBlocks() actually pops the first element of the
  // row block Vector. So the call actually mutates the Vector.
  $row_blocks = $result->rowBlocks();
  if (!$row_blocks->isEmpty()) {
    // An AsyncMysqlRowBlock
    $row_block = $row_blocks[0];
    if (!$row_block->isEmpty()) {
      // An AsyncMysqlRow
      $row = $row_block->getRow(0);
      return $row->getFieldAsDouble('age'); // age int coerced to float
    }
    return -1.0;
  } else {
    return -1.0;
  }
}

<<__EntryPoint>>
async function run(): Awaitable<void> {
  $r = await simple_query();
  \var_dump($r);
}
```.hhvm.expectf
float(%f)
```.example.hhvm.out
float(42)
```.skipif
await \Hack\UserDocumentation\API\Examples\AsyncMysql\skipif_async();
~~~
<!-- HHAPIDOC -->
