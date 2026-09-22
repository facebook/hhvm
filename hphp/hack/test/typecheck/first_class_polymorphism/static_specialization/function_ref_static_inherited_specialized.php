<?hh

class Box<T> {}

class Base<TClass> {
  public static function id<TMethod>(TMethod $value): TMethod {
    return $value;
  }

  public static function box<TMethod>(TClass $_, TMethod $_value): Box<TMethod> {
    return new Box();
  }
}

class Child<TClass> extends Base<vec<TClass>> {}

function take_ref<T>(HH\FunctionRef<T> $_): void {}

function good(): void {
  $explicit = Child::box<string>;
  hh_expect<Box<string>>($explicit(vec[1], 'hello'));
  $inferred = Child::box<_>;
  hh_expect<Box<int>>($inferred(vec['hello'], 1));

  $poly = Child::box<>;
  take_ref($poly);
  hh_expect<Box<int>>($poly(vec['hello'], 1));
  hh_expect<Box<string>>($poly(vec[1], 'hello'));
}

function bad(): void {
  take_ref(Child::id<int>);
  take_ref(Child::box<string>);
  take_ref(Child::box<_>);
}
