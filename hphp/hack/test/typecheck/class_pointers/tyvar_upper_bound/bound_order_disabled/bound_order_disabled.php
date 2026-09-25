<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class C {
  public static function method(): void {}
}

function lower_arrives<T as string>(?T $_ = null): T
where T super class<C> {
  throw new Exception();
}

function upper_arrives<T super class<C> as string>(?T $_ = null): T {
  throw new Exception();
}

function lower_arrives_abstract<T as string>(T $value): T {
  return $value;
}

function test_lower_arrives_abstract(class<C> $class): void {
  $value = lower_arrives_abstract($class);
  $value::method();
}

function test_lower_arrives(): void {
  $value = lower_arrives();
  $value::method();
}

function test_upper_arrives(): void {
  $value = upper_arrives();
  $value::method();
}
