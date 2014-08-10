<?hh

class MyClass {
  private static $privateMSarray = msarray(
    "hello" => true,
    "goodbye" => false,
  );

  public static $publicMSarray = msarray(
    "bonjour" => true,
    "au revoir" => false,
  );
}

function main() {
  MyClass::$publicMSarray[-54] = 'warning';
  MyClass::$publicMSarray[55] = 'no warning';
  var_dump(MyClass::$publicMSarray);
}

main();
