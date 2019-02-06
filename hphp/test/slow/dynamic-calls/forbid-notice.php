<?hh

function wrap($e) { echo "Exception: {$e->getMessage()}\n"; }

function func() {}
async function async_func() { return 5; }
function cmp($x, $y) { return $x <=> $y; }

<<__DynamicallyCallable>> function func2() {}
<<__DynamicallyCallable>> async function async_func2() { return 5; }
<<__DynamicallyCallable>> function cmp2($x, $y) { return $x <=> $y; }

class D {
  public function __call($a, $b) {}
  public static function __callStatic($a, $b) {}
}

class C extends D {
  public function __call($a, $b) {}
  public static function __callStatic($a, $b) {}

  public static function positive_test1() {
    $x = 'foobar';
    try { self::$x(); } catch (Exception $e) { wrap($e); }
    try { static::$x(); } catch (Exception $e) { wrap($e); }
    try { parent::$x(); } catch (Exception $e) { wrap($e); }
  }

  public static function negative_test1() {
    self::foobar();
    static::foobar();
    parent::foobar();
  }
}

class G  {
  <<__DynamicallyCallable>> public function __call($a, $b) {}
  <<__DynamicallyCallable>> public static function __callStatic($a, $b) {}
}

class E extends G {
  <<__DynamicallyCallable>> public function __call($a, $b) {}
  <<__DynamicallyCallable>> public static function __callStatic($a, $b) {}

  public static function negative_test1() {
    $x = 'foobar';
    self::$x();
    static::$x();
    parent::$x();
  }
}

class CCmp {
  public function __call($a, $b) { return $b[0] <=> $b[1]; }
  public static function __callStatic($a, $b) { return $b[0] <=> $b[1]; }
}

class CCmp2 {
  <<__DynamicallyCallable>> public function __call($a, $b) { return $b[0] <=> $b[1]; }
  <<__DynamicallyCallable>> public static function __callStatic($a, $b) { return $b[0] <=> $b[1]; }
}

class Invokable {
  public function __invoke() {}
}

class InvokableCmp {
  public function __invoke($a, $b) { return $a <=> $b; }
}

class B {
  public function func() {}
  public static function static_func() {}
  public async function async_func() { return 5; }
  public static async function static_async_func() { return 5; }

  <<__DynamicallyCallable>> public function func2() {}
  <<__DynamicallyCallable>> public static function static_func2() {}
  <<__DynamicallyCallable>> public async function async_func2() { return 5; }
  <<__DynamicallyCallable>> public static async function static_async_func2() { return 5; }
}

class A extends B {
  public function __construct() {}

  public function func() {}
  public async function async_func() { return 5; }
  public function cmp($x, $y) { return $x <=> $y; }
  public static function static_func() {}
  public static async function static_async_func() { return 5; }
  public static function static_cmp($x, $y) { return $x <=> $y; }

  <<__DynamicallyCallable>> public function func2() {}
  <<__DynamicallyCallable>> public async function async_func2() { return 5; }
  <<__DynamicallyCallable>> public function cmp2($x, $y) { return $x <=> $y; }
  <<__DynamicallyCallable>> public static function static_func2() {}
  <<__DynamicallyCallable>> public static async function static_async_func2() { return 5; }
  <<__DynamicallyCallable>> public static function static_cmp2($x, $y) { return $x <=> $y; }

  public static async function positive_test1() {
    $x = 'func';
    try { self::$x(); } catch (Exception $e) { wrap($e); }
    try { static::$x(); } catch (Exception $e) { wrap($e); }
    try { parent::$x(); } catch (Exception $e) { wrap($e); }

    $x = 'static_func';
    try { self::$x(); } catch (Exception $e) { wrap($e); }
    try { static::$x(); } catch (Exception $e) { wrap($e); }
    try { parent::$x(); } catch (Exception $e) { wrap($e); }

    $x = 'async_func';
    try { await self::$x(); } catch (Exception $e) { wrap($e); }
    try { await static::$x(); } catch (Exception $e) { wrap($e); }
    try { await parent::$x(); } catch (Exception $e) { wrap($e); }

    $x = 'static_async_func';
    try { await self::$x(); } catch (Exception $e) { wrap($e); }
    try { await static::$x(); } catch (Exception $e) { wrap($e); }
    try { await parent::$x(); } catch (Exception $e) { wrap($e); }
  }

  public static async function negative_test1() {
    self::func();
    static::func();
    parent::func();

    self::static_func();
    static::static_func();
    parent::static_func();

    await self::async_func();
    await static::async_func();
    await parent::async_func();

    await self::static_async_func();
    await static::static_async_func();
    await parent::static_async_func();

    new self;
    new static;
    new parent;

    $x = 'func2';
    self::$x();
    static::$x();
    parent::$x();

    $x = 'static_func2';
    self::$x();
    static::$x();
    parent::$x();

    $x = 'async_func2';
    await self::$x();
    await static::$x();
    await parent::$x();

    $x = 'static_async_func2';
    await self::$x();
    await static::$x();
    await parent::$x();

  }
}

<<__DynamicallyConstructible>>
class F extends A {
}

async function positive_tests() {
  try { $x = 'func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'C::foobar'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['C', 'foobar']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new C, 'foobar']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; $x::func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; $x::static_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; await $x::async_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; await $x::static_async_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'C'; $x::foobar(); } catch (Exception $e) { wrap($e); }
  try { $x = 'func'; A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'static_func'; A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'static_async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'foobar'; C::$x(); } catch (Exception $e) { wrap($e); }

  A::positive_test1();
  C::positive_test1();

  try { $x = 'A'; new $x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'func'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'static_func'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'async_func'; await $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'static_async_func';  await $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new C; $x = 'foobar'; $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { array_map('func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('A::func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('A::static_func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('C::foobar', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['A', 'func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['A', 'static_func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['C', 'foobar'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new A, 'func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new A, 'static_func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new C, 'foobar'], [true]); } catch (Exception $e) { wrap($e); }

  $x = Vector::fromItems([[]]);
  try { $x->map('func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('A::func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('A::static_func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('C::foobar'); } catch (Exception $e) { wrap($e); }
  try { $x->map(['A', 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new C, 'foobar']); } catch (Exception $e) { wrap($e); }

  try { call_user_func('func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('A::func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('A::static_func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('C::foobar'); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['A', 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new C, 'foobar']); } catch (Exception $e) { wrap($e); }

  try { call_user_func_array('func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('A::func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('A::static_func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(['A', 'func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(['A', 'static_func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new A, 'func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new A, 'static_func'], []); } catch (Exception $e) { wrap($e); }

  try { $x = 'cmp'; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::cmp'; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_cmp'; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'CCmp::foobar'; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'cmp']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_cmp']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['CCmp', 'foobar']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'cmp']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_cmp']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new CCmp, 'foobar']; $y = [2, 1]; usort(&$y, $x); } catch (Exception $e) { wrap($e); }
}

async function negative_tests() {
  func();
  count([]);
  await async_func();
  A::func();
  A::static_func();
  await A::async_func();
  await A::static_async_func();
  C::foobar();
  Vector::fromItems([]);

  $x = ($k ==> {});
  $x(1);

  A::negative_test1();
  C::negative_test1();
  E::negative_test1();

  new A;
  new Vector;

  $obj = new A;
  $obj->func();
  $obj->static_func();
  await $obj->async_func();
  await $obj->static_async_func();

  $obj = new C;
  $obj->foobar();

  $obj = new Vector;
  $obj->firstValue();
  $obj->fromItems([]);

  array_map($k ==> {}, [true]);
  array_map(new Invokable, [true]);

  $x = Vector::fromItems([true]);
  $x->map($k ==> {});
  $x->map(new Invokable);

  $x = [2, 1];
  usort(&$x, ($k1, $k2) ==> { return $k1 <=> $k2; });

  $x = [2, 1];
  usort(&$x, new InvokableCmp);

  $x = 'count'; $x([]);
  array_map('count', [[]]);
  $x = Vector::fromItems([true]); $x->map('count');
  call_user_func('count', []);
  call_user_func_array('count', [[]]);

  $x = 'HH\Vector::fromItems'; $x([]);
  $x = ['HH\Vector', 'fromItems']; $x([]);
  $x = [new Vector, 'fromItems']; $x([]);
  $x = 'fromItems'; Vector::$x([]);
  $x = [new Vector, 'firstValue']; $x();
  $x = 'HH\Vector'; $x::fromItems([]);
  $obj = new Vector; $x = 'firstValue'; $obj->$x();
  $obj = new Vector; $x = 'fromItems'; $obj->$x([]);
  array_map('HH\Vector::fromItems', [[]]);
  array_map(['HH\Vector', 'fromItems'], [[]]);
  array_map([new Vector, 'fromItems'], [[]]);
  $x = Vector::fromItems([[]]);
  $x->map('HH\Vector::fromItems');
  $x->map(['HH\Vector', 'fromItems']);
  $x->map([new Vector, 'fromItems']);
  call_user_func('HH\Vector::fromItems', []);
  call_user_func(['HH\Vector', 'fromItems'], []);
  call_user_func([new Vector, 'firstValue']);
  call_user_func([new Vector, 'fromItems'], []);
  call_user_func_array('HH\Vector::fromItems', [[]]);
  call_user_func_array(['HH\Vector', 'fromItems'], [[]]);
  call_user_func_array([new Vector, 'firstValue'], []);
  call_user_func_array([new Vector, 'fromItems'], [[]]);

  $x = 'array_map';
  $x('count', []);

  $obj = null; $obj?->foobar();

  idx('foobar', 'key');

  $x = 'func2'; $x();
  $x = 'async_func2'; await $x();
  $x = 'A::func2'; $x();
  $x = 'A::static_func2'; $x();
  $x = 'A::async_func2'; await $x();
  $x = 'A::static_async_func2'; await $x();
  $x = 'E::foobar'; $x();
  $x = ['A', 'func2']; $x();
  $x = ['A', 'static_func2']; $x();
  $x = ['A', 'async_func2']; await $x();
  $x = ['A', 'static_async_func2']; await $x();
  $x = ['E', 'foobar']; $x();
  $x = [new A, 'func2']; $x();
  $x = [new A, 'static_func2']; $x();
  $x = [new A, 'async_func2']; await $x();
  $x = [new A, 'static_async_func2']; await $x();
  $x = [new E, 'foobar']; $x();
  $x = 'A'; $x::func2();
  $x = 'A'; $x::static_func2();
  $x = 'A'; await $x::async_func2();
  $x = 'A'; await $x::static_async_func2();
  $x = 'E'; $x::foobar();
  $x = 'func2'; A::$x();
  $x = 'static_func2'; A::$x();
  $x = 'async_func2'; await A::$x();
  $x = 'static_async_func2'; await A::$x();
  $x = 'foobar'; E::$x();

  $x = 'F'; new $x();
  $obj = new A; $x = 'func2'; $obj->$x();
  $obj = new A; $x = 'static_func2'; $obj->$x();
  $obj = new A; $x = 'async_func2'; await $obj->$x();
  $obj = new A; $x = 'static_async_func2';  await $obj->$x();
  $obj = new E; $x = 'foobar'; $obj->$x();

  array_map('func2', [true]);
  array_map('A::func2', [true]);
  array_map('A::static_func2', [true]);
  array_map('E::foobar', [true]);
  array_map(['A', 'func2'], [true]);
  array_map(['A', 'static_func2'], [true]);
  array_map(['E', 'foobar'], [true]);
  array_map([new A, 'func2'], [true]);
  array_map([new A, 'static_func2'], [true]);
  array_map([new E, 'foobar'], [true]);

  $x = Vector::fromItems([[]]);
  $x->map('func2');
  $x->map('A::func2');
  $x->map('A::static_func2');
  $x->map('E::foobar');
  $x->map(['A', 'func2']);
  $x->map(['A', 'static_func2']);
  $x->map(['E', 'foobar']);
  $x->map([new A, 'func2']);
  $x->map([new A, 'static_func2']);
  $x->map([new E, 'foobar']);

  call_user_func('func2');
  call_user_func('A::func2');
  call_user_func('A::static_func2');
  call_user_func('E::foobar');
  call_user_func(['A', 'func2']);
  call_user_func(['A', 'static_func2']);
  call_user_func(['E', 'foobar']);
  call_user_func([new A, 'func2']);
  call_user_func([new A, 'static_func2']);
  call_user_func([new E, 'foobar']);

  call_user_func_array('func2', []);
  call_user_func_array('A::func2', []);
  call_user_func_array('A::static_func2', []);
  call_user_func_array(['A', 'func2'], []);
  call_user_func_array(['A', 'static_func2'], []);
  call_user_func_array([new A, 'func2'], []);
  call_user_func_array([new A, 'static_func2'], []);

  $x = 'cmp2'; $y = [2, 1]; usort(&$y, $x);
  $x = 'A::cmp2'; $y = [2, 1]; usort(&$y, $x);
  $x = 'A::static_cmp2'; $y = [2, 1]; usort(&$y, $x);
  $x = 'CCmp2::foobar'; $y = [2, 1]; usort(&$y, $x);
  $x = ['A', 'cmp2']; $y = [2, 1]; usort(&$y, $x);
  $x = ['A', 'static_cmp2']; $y = [2, 1]; usort(&$y, $x);
  $x = ['CCmp2', 'foobar']; $y = [2, 1]; usort(&$y, $x);
  $x = [new A, 'cmp2']; $y = [2, 1]; usort(&$y, $x);
  $x = [new A, 'static_cmp2']; $y = [2, 1]; usort(&$y, $x);
  $x = [new CCmp2, 'foobar']; $y = [2, 1]; usort(&$y, $x);
}

echo "=============== positive tests =====================\n";
HH\Asio\join(positive_tests());

echo "=============== negative tests =====================\n";
HH\Asio\join(negative_tests());

set_error_handler(
  function ($type, $msg, $file) { throw new Exception($msg); }
);
echo "=============== positive tests (exceptions) ========\n";
HH\Asio\join(positive_tests());
