<?hh

function id<T>(T $value): T {
  return $value;
}

function int_id(int $value): int {
  return $value;
}

function int_result()[]: int {
  return 1;
}

class Identity {
  public static function id<T>(T $value): T {
    return $value;
  }

  public static function mono(int $value): int {
    return $value;
  }

  public static function apply((function()[_]: int) $f)[ctx $f]: int {
    return $f();
  }
}

function take_ref<T>(HH\FunctionRef<T> $_): void {}

function mock_function<T>(HH\FunctionRef<T> $_original, T $_replacement): void {}

function good(): void {
  $explicit = Identity::id<int>;
  hh_expect<int>($explicit(1));
  $inferred = Identity::id<_>;
  hh_expect<int>($inferred(2));

  $poly = Identity::id<>;
  hh_expect<int>($poly(1));
  hh_expect<string>($poly('hello'));
  take_ref($poly);
  mock_function($poly, id<>);
  take_ref(Identity::mono<>);

  $coeffects = Identity::apply<>;
  take_ref($coeffects);
  hh_expect<int>($coeffects(int_result<>));
}

function bad(): void {
  take_ref(Identity::id<int>);
  take_ref(Identity::id<_>);
  mock_function(Identity::id<int>, int_id<>);
  mock_function(Identity::id<_>, int_id<>);
  mock_function(Identity::id<>, int_id<>);
  take_ref(id<int>);
  take_ref(id<_>);
}
