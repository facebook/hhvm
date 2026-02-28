<?hh
<<__SimpliHack(prompt(nameof SomeClass))>>
class SomeClass<T1, T2> {
  public function __construct(
    public int $one,
    public vec<string> $two,
    public T1 $three,
    public ?T2 $four = null,
  ) {}

  public function getOne(): int { return $this->one; }
  public function getTwo(): vec<string> { return $this->two; }
  public function getThree(): T1 { return $this->three; }
  public function getFour(): ?T2 { return $this->four; }

  public static function getFive(): int { return 5; }
  public static function getSix(): vec<string> { return vec['six']; }
}

function prompt(classname<mixed> $cls)[HH\SimpliHack]: string {
    $constructor = HH\SimpliHack\constructor($cls);
    $methods = HH\SimpliHack\methods($cls);
    $static_methods = HH\SimpliHack\static_methods($cls);
    $fields = HH\SimpliHack\fields($cls);
    $static_fields = HH\SimpliHack\static_fields($cls);
    return "
      constructor: {$constructor}
      methods: {$methods}
      static methods: {$static_methods}
      fields: {$fields}
      static fields: {$static_fields}
    "
}
