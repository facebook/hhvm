<?hh

class A {
  function __construct($arg) {
    $this->a = $arg;
  }

  static function bar() : void  {
    echo "In bar\n";
  }

  public int $a;
}

class C {
  public static function rclsmeth<reify T>() :mixed{}
}

function rfunc<reify T>() :mixed{}

function foo() : void {
  echo "In foo\n";
}

enum class E : int {
  int A = 42;
  int B = 45;
}

// lazy class
abstract class Controller {
  const type TC = int;
  public static function foo() {
    return type_structure(static::class, 'TC');
  }
}

function getResource() :mixed{
  return fopen(__FILE__, "r");
}

function testConvToInt() : void {
  $shard_num = __hhvm_intrinsics\launder_value(8);
  $schema = __hhvm_intrinsics\launder_value("schema");

  // Integers
  echo HH\Lib\Str\format("8: %s%d\n", $schema, __hhvm_intrinsics\launder_value(8));
  echo HH\Lib\Str\format("-1: %s%d\n", $schema, __hhvm_intrinsics\launder_value(-1));
  echo HH\Lib\Str\format("+1: %s%d\n", $schema, __hhvm_intrinsics\launder_value(+1));
  // Double
  echo HH\Lib\Str\format("0.02: %s%d\n", $schema, __hhvm_intrinsics\launder_value(0.02));
  echo HH\Lib\Str\format("-0.02: %s%d\n", $schema, __hhvm_intrinsics\launder_value(-0.02));
  echo HH\Lib\Str\format("+0.02: %s%d\n", $schema, __hhvm_intrinsics\launder_value(+0.02));

  // String
  echo HH\Lib\Str\format("Persistent Str \"8\": %s%d\n", $schema, "8");
  echo HH\Lib\Str\format("\"8\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("8"));
  echo HH\Lib\Str\format("\"-1\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("-1"));
  echo HH\Lib\Str\format("\"008\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("008"));
  echo HH\Lib\Str\format("\"+8\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("+8"));
  echo HH\Lib\Str\format("\"0.8\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("0.8"));
  echo HH\Lib\Str\format("\"-0.8\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("-0.8"));
  echo HH\Lib\Str\format("\"badint\": %s%d\n", $schema, __hhvm_intrinsics\launder_value("badint"));

  // Null
  echo HH\Lib\Str\format("null: %s%d\n", $schema, __hhvm_intrinsics\launder_value(null));

  // bool
  echo HH\Lib\Str\format("true: %s%d\n", $schema, __hhvm_intrinsics\launder_value(true));
  echo HH\Lib\Str\format("false: %s%d\n", $schema, __hhvm_intrinsics\launder_value(false));

  // containers
  echo HH\Lib\Str\format(
    "Persistent vec[1,2]: %s%d\n",
    $schema,
    vec[1, 2]
  );
  echo HH\Lib\Str\format(
    "vec[1,2]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(__hhvm_intrinsics\launder_value(vec[1,2]))
  );
  echo HH\Lib\Str\format(
    "Persistent dict[\"1\" => 2]: %s%d\n",
    $schema,
    dict["1" => 2]
  );
  echo HH\Lib\Str\format(
    "dict[\"1\" => 2]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(dict["1" => 2])
  );
  echo HH\Lib\Str\format(
    "Persistent keyset[\"1\"]: %s%d\n",
    $schema,
    keyset["1"]
  );
  echo HH\Lib\Str\format(
    "keyset[\"1\"]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(keyset["1"])
  );
  echo HH\Lib\Str\format(
    "vec[]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(vec[])
  );
  echo HH\Lib\Str\format(
    "dict[]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(dict[])
  );
  echo HH\Lib\Str\format(
    "keyset[]: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(keyset[])
  );

  // Enums
  echo HH\Lib\Str\format(
    "enum: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(E::A)
  );

  try {
    echo HH\Lib\Str\format(
      "enum: %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(E#A)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // objects
  try {
    echo HH\Lib\Str\format(
      "new A(1): %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(new A(1))
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // functions
  try {
    echo HH\Lib\Str\format(
      "function: %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(foo<>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // class
  echo HH\Lib\Str\format(
    "class: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(A::class)
  );

  // Lazy class
  $cls = Controller::foo();
  echo HH\Lib\Str\format(
    "lazycls: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value($cls)
  );

  // Resource
  echo HH\Lib\Str\format(
    "resource: %s%d\n",
    $schema,
    __hhvm_intrinsics\launder_value(getResource())
  );

  // class method
  try {
    echo HH\Lib\Str\format(
      "method: %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(A::bar<>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // reified generics
  try {
    echo HH\Lib\Str\format(
      "rclsmeth: %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(C::rclsmeth<int>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  try {
    echo HH\Lib\Str\format(
      "rfunc: %s%d\n",
      $schema,
      __hhvm_intrinsics\launder_value(rfunc<int>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // Done
  echo "Done\n";
}

function testConvToDbl() : void {
  $shard_num = __hhvm_intrinsics\launder_value(8);
  $schema = __hhvm_intrinsics\launder_value("schema");

  // Integers
  echo HH\Lib\Str\format("8: %s%f\n", $schema, __hhvm_intrinsics\launder_value(8));
  echo HH\Lib\Str\format("-1: %s%f\n", $schema, __hhvm_intrinsics\launder_value(-1));
  echo HH\Lib\Str\format("+1: %s%f\n", $schema, __hhvm_intrinsics\launder_value(+1));
  // Double
  echo HH\Lib\Str\format("0.02: %s%f\n", $schema, __hhvm_intrinsics\launder_value(0.02));
  echo HH\Lib\Str\format("-0.02: %s%f\n", $schema, __hhvm_intrinsics\launder_value(-0.02));
  echo HH\Lib\Str\format("+0.02: %s%f\n", $schema, __hhvm_intrinsics\launder_value(+0.02));

  // String
  echo HH\Lib\Str\format("Persistent Str \"8\": %s%d\n", $schema, "8");
  echo HH\Lib\Str\format("\"8\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("8"));
  echo HH\Lib\Str\format("\"-1\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("-1"));
  echo HH\Lib\Str\format("\"008\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("008"));
  echo HH\Lib\Str\format("\"+8\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("+8"));
  echo HH\Lib\Str\format("\"0.8\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("0.8"));
  echo HH\Lib\Str\format("\"-0.8\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("-0.8"));
  echo HH\Lib\Str\format("\"badint\": %s%f\n", $schema, __hhvm_intrinsics\launder_value("badint"));

  // Null
  echo HH\Lib\Str\format("null: %s%f\n", $schema, __hhvm_intrinsics\launder_value(null));

  // bool
  echo HH\Lib\Str\format("true: %s%f\n", $schema, __hhvm_intrinsics\launder_value(true));
  echo HH\Lib\Str\format("false: %s%f\n", $schema, __hhvm_intrinsics\launder_value(false));

  // containers
  echo HH\Lib\Str\format(
    "Persistent vec[1,2]: %s%f\n",
    $schema,
    vec[1, 2]
  );
  echo HH\Lib\Str\format(
    "vec[1,2]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(__hhvm_intrinsics\launder_value(vec[1,2]))
  );
  echo HH\Lib\Str\format(
    "Persistent dict[\"1\" => 2]: %s%f\n",
    $schema,
    dict["1" => 2]
  );
  echo HH\Lib\Str\format(
    "dict[\"1\" => 2]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(dict["1" => 2])
  );
  echo HH\Lib\Str\format(
    "Persistent keyset[\"1\"]: %s%f\n",
    $schema,
    keyset["1"]
  );
  echo HH\Lib\Str\format(
    "keyset[\"1\"]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(keyset["1"])
  );
  echo HH\Lib\Str\format(
    "vec[]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(vec[])
  );
  echo HH\Lib\Str\format(
    "dict[]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(dict[])
  );
  echo HH\Lib\Str\format(
    "keyset[]: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(keyset[])
  );

  // Enums
  echo HH\Lib\Str\format(
    "enum: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(E::A)
  );

  try {
    echo HH\Lib\Str\format(
      "enum: %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(E#A)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // objects
  try {
    echo HH\Lib\Str\format(
      "new A(1): %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(new A(1))
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // functions
  try {
    echo HH\Lib\Str\format(
      "function: %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(foo<>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // class
  echo HH\Lib\Str\format(
    "class: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(A::class)
  );

  // Lazy class
  $cls = Controller::foo();
  echo HH\Lib\Str\format(
    "lazycls: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value($cls)
  );

  // Resource
  echo HH\Lib\Str\format(
    "resource: %s%f\n",
    $schema,
    __hhvm_intrinsics\launder_value(getResource())
  );

  // class method
  try {
    echo HH\Lib\Str\format(
      "method: %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(A::bar<>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // reified generics
  try {
    echo HH\Lib\Str\format(
      "rclsmeth: %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(C::rclsmeth<int>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  try {
    echo HH\Lib\Str\format(
      "rfunc: %s%f\n",
      $schema,
      __hhvm_intrinsics\launder_value(rfunc<int>)
    );
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}

  // Done
  echo "Done\n";
}

<<__EntryPoint>>
function main() {
  testConvToInt();
  testConvToDbl();
}
