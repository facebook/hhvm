<?hh

class A {
  private $data;

  public function __construct($data) {
    $this->data = $data;
  }

  <<__DynamicallyCallable>>
  public function M1() :mixed{
    return $this->data;
  }

  <<__DynamicallyCallable>>
  protected function M2() :mixed{
    return $this->data;
  }

  <<__DynamicallyCallable>>
  private function M3() :mixed{
    return $this->data;
  }

  <<__DynamicallyCallable>>
  static function M4() :mixed{
    return 'static';
  }
}

class B { }

function execute($class_name, $method_name, $instance) :mixed{
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
function main_get_closure_instance() :mixed{
execute('A', 'M1', new A(true));
execute('A', 'M2', new A(42));
execute('A', 'M3', new A(vec[]));
execute('A', 'M4', new A('NOT STATIC'));

execute('A', 'M1', new B());
execute('A', 'M2', new B());
execute('A', 'M3', new B());
execute('A', 'M4', new B('NOT STATIC'));
}
