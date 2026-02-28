<?hh

enum MyStringEnum: string as string {
  A = 'a';
}

interface IMyInterface {
  abstract const type T;
}

final class HasStringEnum implements IMyInterface {
  const type T = MyStringEnum;
}

final class MyClass {
  public function getEnum(): MyStringEnum {
    throw new Exception();
  }
  public function getNullableEnum(): ?MyStringEnum {
    throw new Exception();
  }

  public function takes<Tval>(
    IMyInterface with { type T super Tval } $obj,
    Tval $val,
  ): void {
    throw new Exception();
  }
}

function mymain(HH\Lib\Ref<MyClass> $r): void {
  $obj = $r->value;
  $nullable_enum = $obj->getNullableEnum();
  if ($nullable_enum is null) {
  } else {
    if ($obj->getEnum() === $nullable_enum) {
    }
    $obj->takes(new HasStringEnum(), $nullable_enum);
  }
}
