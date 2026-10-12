<?hh

enum ReproEnum: int {
  A = 1;
}

abstract class ReproFieldBase {
  abstract public function getTypeAbs(): string;
  public function getType(): string {
    return '';
  }
}

final class ReproClassField extends ReproFieldBase {
  public function __construct(private class<mixed> $classType) {}

  <<__Override>>
  public function getTypeAbs(): class<mixed> {
    return $this->classType;
  }

  <<__Override>>
  public function getType(): class<mixed> {
    return $this->classType;
  }
}
