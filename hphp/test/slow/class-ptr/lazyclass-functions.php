<?hh

final class Foo {
  public static string $publicStatic = "hello";
  protected static string $protectedStatic = "world";
  private static string $privateStatic= "hello";
  public static string $publicInstance = "world";
  protected static string $protectedInstance = "hello";
  private static string $privateInstance = "world";
}

abstract class Bar {
  public static string $publicStatic = "hello";
  protected static string $protectedStatic = "world";
  private static string $privateStatic= "hello";
  public static string $publicInstance = "world";
  protected static string $protectedInstance = "hello";
  private static string $privateInstance = "world";
}

<<__EntryPoint>>
function main() :mixed{
  $classes = vec[Foo::class, Bar::class];
  foreach ($classes as $to_test) {
    var_dump($to_test);
    var_dump(property_exists($to_test, "publicStatic"));
    var_dump(property_exists($to_test, "protectedStatic"));
    var_dump(property_exists($to_test, "privateStatic"));
    var_dump(property_exists($to_test, "publicInstance"));
    var_dump(property_exists($to_test, "protectedInstance"));
    var_dump(property_exists($to_test, "privateInstance"));
    var_dump(property_exists($to_test, "propertyThatDoesNotExist"));
  }
}
