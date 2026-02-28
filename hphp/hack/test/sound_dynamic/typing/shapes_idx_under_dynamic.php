<?hh

<<__SupportDynamicType>>
class C {

  private ?~self::TS $prop = null;

  const type TS = shape(
    ?'a' => ?int,
    ?'b' => ?string,
  );

  public function getit():~self::TS {
    throw new Exception("A");
  }

  public function foo(
    vec<int> $vi,
  ): void {
    $fields = $this->getit();
    if ($this->prop is null) {
      $this->prop = shape();
    }
    $this->prop['a'] = Shapes::idx($fields,'a');
    // This fails under dynamic assumptions because we're assigning to ~null
    $this->prop['b'] = Shapes::idx($fields, 'b');
  }
}
