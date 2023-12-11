<?hh

abstract final class TestAsyncMysqlConn {
  public static $conn = null;
  public static function testMultiQuery($input) :mixed{
    if (self::$conn === null) {
      $conn = (new ReflectionClass(AsyncMysqlConnection::class))
        ->newInstanceWithoutConstructor();
    }
    try {
      \HH\Asio\join($conn->multiQuery($input));
    } catch (Exception $_) {}
  }
}
<<__EntryPoint>> function main(): void {
$pool = new AsyncMysqlConnectionPool(dict['foo' => null]);
var_dump($pool->getPoolStats());

$inputs = vec[
  vec[],
  dict[],
  Vector {},
  dict[],
  vec[],
];

foreach ($inputs as $i) {
  TestAsyncMysqlConn::testMultiQuery($i);
}
}
