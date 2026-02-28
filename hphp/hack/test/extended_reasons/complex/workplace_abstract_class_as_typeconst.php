<?hh

abstract class ContainerClass {
  abstract const type TType as arraykey;
  public function add(this::TType $val): void {}
}

abstract class ExampleClass {
  abstract const type TContainerClass as ContainerClass;

  protected ?this::TContainerClass $container;

  public function addToContainer(this::TContainerClass::TType $val): void {
    $this->container?->add($val);
  }
}

final class IntContainerClass extends ContainerClass {
  const type TType = int;
}

final class ExampleIntClass extends ExampleClass {
  const type TContainerClass = IntContainerClass;
}

function test_function_1(): void {
  $i = new ExampleIntClass();
  $i->addToContainer(1);
}

final class ExampleArraykeyClass extends ExampleClass {
  const type TContainerClass = ContainerClass;
}

function test_function_2(): void {
  $ak = new ExampleArraykeyClass();
  $ak->addToContainer(1);
}
