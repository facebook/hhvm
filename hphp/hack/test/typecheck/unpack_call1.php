<?hh

function f(int $foo, ...$args): void {}

function g(string $foo, ...$args): void {}

class C1 {
  public function __construct(
    private int $foo,
    private ...$args
  ) {}
  public function f(int $foo, ...$args): void {}
}

class C2 extends C1 {
  public function __construct(
    private int $foo,
    private ...$args
  ) {
    parent::__construct($foo, ...$args);
  }
}

function make_int_args(): Container<int> {
  // UNSAFE
}

function make_mixed_args(): array<mixed> {
  // UNSAFE
}

function make_str_args(): Vector<string> {
  // UNSAFE
}

function test_basic(): void {
  $args = array(1, 2, 3);
  f(...$args);
  $inst = new C1(...$args);
  $inst->f(...$args);
  $inst = new C2(...$args);
  $inst->f(...$args);

  switch ( mt_rand() % 5 ) {
    case 0: $args[] = 20; break;
    case 1: $args[] = 'string'; $args[] = Vector {}; break;
    case 2: $args[] = new stdClass(); break;
    default: break;
  }

  f(...make_int_args());
  $make = fun('make_int_args');
  f(...$make());

  f(...make_mixed_args());
  $make = fun('make_mixed_args');
  f(...$make());

  g(
    'this is a long string to satisfy hh_format. it allows us to test that ',
    'trailing commas after unpacked calls will parse',
    ...$args,
  );
}

function test_limitations() {
  // fails at runtime, but we don't track array arity!
  $args = array();
  f(...$args);

  // fails at runtime, but we don't ensure that container doesn't have
  // string keys; because it's laborious to check for
  // Container-but-not-KeyedContainer<string, >
  $args['a'] = 1;
  f(...$args);

  $args = Map { 'a' => 1, 'b' => 2, };
  f(1, ...$args);

  $args = array( 'a' => 1, 'b' => 2 );
  f(1, ...$args);

  // fails at runtime, but we don't unpack the container's content type
  // because issuing errors for mixed pretty much destroys the feature
  f(...make_string_args());
  $make = fun('make_string_args');
  f(...$make());
}
