<?hh

interface Marker {}

class Base {
  public static function id<T>(T $value): T {
    return $value;
  }
}

function take_ref<T>(HH\FunctionRef<T> $_): void {}

function good<reify TClass as Base as Marker>(): void {
  $explicit = TClass::id<int>;
  hh_expect<int>($explicit(1));
  $inferred = TClass::id<_>;
  hh_expect<int>($inferred(2));

  $poly = TClass::id<>;
  take_ref($poly);
  hh_expect<int>($poly(1));
  hh_expect<string>($poly('hello'));
}

function bad<reify TClass as Base as Marker>(): void {
  take_ref(TClass::id<int>);
  take_ref(TClass::id<_>);
}
