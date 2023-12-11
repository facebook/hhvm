<?hh

class Bar {
  public static function foo(): void {}
}

<<__EntryPoint>>
function main(): void {

  $clsmeth = Bar::foo<>;

  $arr1 = vec[];
  $arr2 = vec[99];
  $arr3 = vec['foo'];
  $arr4 = vec['foo', 'bar'];
  $arr5 = vec['foo', 42];

  echo $clsmeth == $arr1 ? "clsmeth == arry1\n" : "clsmeth != arry1\n";
  echo $clsmeth == $arr2 ? "clsmeth == arry2\n" : "clsmeth != arry2\n";
  echo $clsmeth == $arr3 ? "clsmeth == arry3\n" : "clsmeth != arry3\n";
  echo $clsmeth == $arr4 ? "clsmeth == arry4\n" : "clsmeth != arry4\n";
  echo $clsmeth == $arr5 ? "clsmeth == arry5\n" : "clsmeth != arry5\n";
  echo "\n";

  $arr2 = vec[99, 'foo'];
  $arr3 = vec['Bar'];
  $arr4 = vec['Bar', 'foo'];
  $arr5 = vec['Bar', 0];

  $try = ($a, $b, $num) ==> {
    try {
      echo $a != $b ? "clsmeth != arry$num\n" : "clsmeth == arry$num\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
  };

  $try($clsmeth, $arr1, 1);
  $try($clsmeth, $arr2, 2);
  $try($clsmeth, $arr3, 3);
  $try($clsmeth, $arr4, 4);
  $try($clsmeth, $arr5, 5);

}
