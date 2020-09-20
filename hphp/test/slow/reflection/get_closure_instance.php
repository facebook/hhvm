<?hh

class A {
  private $data;

  public function __construct($data) {
    $this->data = $data;
  }

  public function M1() {
    return $this->data;
  }

  protected function M2() {
    return $this->data;
  }

  private function M3() {
    return $this->data;
  }

  static function M4() {
    return 'static';
  }
}

class B { }

function execute($class_name, $method_name, $instance) {
  try {
    $ref = new ReflectionMethod($class_name, $method_name);
    $ref->setAccessible(true);
    $method = $ref->getClosure($instance);
    var_dump($method());
  } catch (Exception $e) {
    $c = get_class($e);
    echo "$c: {$e->getMessage()}\n";
  }
}


<<__EntryPoint>>
function main_get_closure_instance() {
execute('A', 'M1', new A(true));
execute('A', 'M2', new A(42));
execute('A', 'M3', new A(varray[]));
execute('A', 'M4', new A('NOT STATIC'));

execute('A', 'M1', new B());
execute('A', 'M2', new B());
execute('A', 'M3', new B());
execute('A', 'M4', new B('NOT STATIC'));
}
