<?hh

class A {
  public $a = 1;
  private $b = "hello";
  protected $c = vec[1, 2];
}

class B extends A {
  public $b = 0;
}

class C {
  public $a, $b, $c;
  function __construct() {
    $this->a = null;
    $this->b = acos(1.01);
    $this->c = log(0.0);
    echo "C has a safe constructor.\n";
  }
  function __wakeup() :mixed{
    echo "C wakes up safely.\n";
  }
  function __sleep() :mixed{
    echo "C sleeps safely.\n";
    return vec['a', 'b', 'c'];
  }
}

class DangerousClass {
  public $danger = "DangerousString";
  function __construct() {
    echo "I have dangerous constructor.\n";
  }
  function __wakeup() :mixed{
    echo "I wake up dangerously.\n";
  }
  function __sleep() :mixed{
    echo "I sleep dangerously.\n";
    return vec['danger'];
  }
}

class E {
  public $dangerousClass;
  function __construct() {
    $this->dangerousClass = new DangerousClass;
  }
}

class F implements Serializable {
  public function serialize() :mixed{
    return "SerializedData";
  }
  public function unserialize($serialized) :mixed{
    echo "unserialize: $serialized\n";
    return $this;
  }
}

class G extends DangerousClass {
}

function test_serialization($obj, $class_whitelist) :mixed{
  $str = serialize($obj);
  var_dump($str);
  $new_obj = unserialize($str, darray($class_whitelist));
  var_dump($new_obj);
  unset($obj);
  unset($new_obj);
  echo "========================\n";
}

<<__EntryPoint>> function main(): void {
  test_serialization(new A, vec[]);
  test_serialization(new B, vec['A', 'B']);
  test_serialization(new C, vec['C']);
  test_serialization(new DangerousClass, vec[]);
  test_serialization(new E, vec['E']);
  test_serialization(new F, vec['F']);
  test_serialization(new G, vec['G']);
  test_serialization(vec["Hello World<>$%", acos(1.01), log(0.0), 50], vec[]);
  test_serialization(
    vec[ new A, vec[new B, vec[new C, vec[new E, vec[new F]]]]],
    dict[
      'abc' => 'A',
      5 => 'C',
      6 => 'E',
      7 => 'B',
      8 => 'F'
    ]
  );
}
