<?hh

class ParentClass {}
class ChildClass extends ParentClass {}

abstract class Base {
  abstract public function getClassname(): classname<mixed>;
  abstract public function takeClass(class<mixed> $value): void;
  abstract public function getClass(): class<ParentClass>;
  abstract public function takeChildClass(class<ChildClass> $value): void;
}

final class Child extends Base {
  <<__Override>>
  public function getClassname(): class<mixed> {
    throw new Exception();
  }

  <<__Override>>
  public function takeClass(classname<mixed> $value): void {}

  <<__Override>>
  public function getClass(): class<ChildClass> {
    return ChildClass::class;
  }

  <<__Override>>
  public function takeChildClass(class<ParentClass> $value): void {}
}

function as_classname(class<mixed> $value): classname<mixed> {
  return $value;
}
