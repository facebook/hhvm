<?hh

function f(int $foo, mixed ...$args): void {}

function g(string $foo, mixed ...$args): void {}

class C1 {
  public function __construct(
    private int $foo,
    private mixed ...$args
  ) {}
  public function f(int $foo, mixed ...$args): void {}
}

class C2 extends C1 {
  public function __construct(
    private int $foo,
    private mixed ...$args
  ) {
    parent::__construct($foo, ...$args);
  }
}

function make_int_args(): Container<int> {
  return vec[];
}

function make_mixed_args(): varray<mixed> {
  return vec[];
}

function make_str_args(): Vector<string> {
  return Vector {};
}

function test_basic(): void {
  $args = vec[1, 2, 3];
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
  $make = make_int_args<>;
  f(...$make());

  f(1, ...make_mixed_args());
  $make = make_mixed_args<>;
  f(1, ...$make());

  g(
    'this is a long string to satisfy hh_format. it allows us to test that ',
    'trailing commas after unpacked calls will parse',
    ...$args,
  );
}

function test_limitations(): void {
  // fails at runtime, but we don't track array arity!
  $args = dict[];
  f(...$args);

  // fails at runtime, but we don't ensure that container doesn't have
  // string keys; because it's laborious to check for
  // Container-but-not-KeyedContainer<string, >
  $args['a'] = 1;
  f(...$args);

  $args = Map { 'a' => 1, 'b' => 2, };
  f(1, ...$args);

  $args = dict[ 'a' => 1, 'b' => 2 ];
  f(1, ...$args);

  // fails at runtime, but we don't unpack the container's content type
  // because issuing errors for mixed pretty much destroys the feature
  f(...make_mixed());
  $make = make_mixed<>;
  f(...$make());
}

function make_mixed(): mixed {
  throw new Exception('');
}
