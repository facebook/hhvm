<?hh

abstract class Base {
  abstract public function getType(): string;
  abstract public function getClassname(): classname<mixed>;
  abstract public function takeClass(class<mixed> $value): void;
}

final class Child extends Base {
  <<__Override>>
  public function getType(): class<mixed> {
    throw new Exception();
  }

  <<__Override>>
  public function getClassname(): class<mixed> {
    throw new Exception();
  }

  <<__Override>>
  public function takeClass(classname<mixed> $value): void {}
}
