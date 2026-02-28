<?hh

abstract enum class Base: mixed {
  float Float = 0.0;
}

enum class Enum1 : mixed extends Base {
  int Int = 1;
}

enum class Enum2 : mixed extends Enum1 {
  bool Bool = true;
}

class EnumClassWrapper<Tclass> {
  public function __construct(
    private classname<HH\GenericEnumClass<Tclass, mixed>> $cls,
  ){}

  public function valueOf<TType>(
    \HH\EnumClass\Label<Tclass, TType> $label,
  )[write_props]: HH\MemberOf<Tclass, TType> {
    $cls = $this->cls;
    return $cls::valueOf($label);
  }
}

function test(
  EnumClassWrapper<Base> $base,
  EnumClassWrapper<Enum1> $e1,
  EnumClassWrapper<Enum2> $e2,
): void {
  hh_expect<float>($base->valueOf(#Float));
  hh_expect<float>($e1->valueOf(#Float));
  hh_expect<float>($e2->valueOf(#Float));

  $base->valueOf(#Int); // Error
  hh_expect<int>($e1->valueOf(#Int));
  hh_expect<int>($e2->valueOf(#Int));

  $base->valueOf(#Bool); // Error
  $e1->valueOf(#Bool); // Error
  hh_expect<bool>($e2->valueOf(#Bool));
}
