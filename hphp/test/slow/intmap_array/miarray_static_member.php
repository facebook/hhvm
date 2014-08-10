<?hh

class MyClass {
  private static $privateMIArray = miarray(
    -1 => "hello",
    45 => "goodbye",
  );

  public static $publicMIArray = miarray(
    7 => 10,
    10 => 13,
  );
}

function main() {
  MyClass::$publicMIArray['foo'] = 'warning';
  MyClass::$publicMIArray['bar'] = 'no warning';
  var_dump(MyClass::$publicMIArray);
}

main();
