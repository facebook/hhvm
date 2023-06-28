<?hh

function Test()
:mixed{
    $c = 1;
    Lang007::$b = 5;
    echo TestStatics::$a." ".Lang007::$b." ";
    TestStatics::$a++;
    $c++;
    echo TestStatics::$a." $c ";
}

abstract final class TestStatics {
  public static $a = 1;
}

abstract final class Lang007 {
  public static $b;
}

<<__EntryPoint>> function main(): void {
error_reporting(0);
$a = 10;

Test();
try {
  echo "$a ".Lang007::$b." $c ";
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
Test();
try {
  echo "$a ".Lang007::$b." $c ";
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
Test();
}
