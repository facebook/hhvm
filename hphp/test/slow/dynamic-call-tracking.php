<?hh

function wrap($e) { echo "Exception: {$e->getMessage()}\n"; }

function func() {}
async function async_func() { return 5; }
function cmp($x, $y) { return $x <=> $y; }

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

  public static function positive_test2() {
    try { forward_static_call('C::foobar'); } catch (Exception $e) { wrap($e); }
    try { forward_static_call(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
    try { forward_static_call([new C, 'foobar']); } catch (Exception $e) { wrap($e); }
  }

  public static function negative_test1() {
    self::foobar();
    static::foobar();
    parent::foobar();
  }
}

class CCmp {
  public function __call($a, $b) { return $b[0] <=> $b[1]; }
  public static function __callStatic($a, $b) { return $b[0] <=> $b[1]; }
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
}

class A extends B {
  public function func() {}
  public async function async_func() { return 5; }
  public function cmp($x, $y) { return $x <=> $y; }
  public static function static_func() {}
  public static async function static_async_func() { return 5; }
  public static function static_cmp($x, $y) { return $x <=> $y; }

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

  public static function positive_test2() {
    try { forward_static_call('func'); } catch (Exception $e) { wrap($e); }
    try { forward_static_call('count', []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call('A::func'); } catch (Exception $e) { wrap($e); }
    try { forward_static_call('A::static_func'); } catch (Exception $e) { wrap($e); }
    try { forward_static_call(['A', 'func']); } catch (Exception $e) { wrap($e); }
    try { forward_static_call(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
    try { forward_static_call([new A, 'func']); } catch (Exception $e) { wrap($e); }
    try { forward_static_call([new A, 'static_func']); } catch (Exception $e) { wrap($e); }

    try { forward_static_call_array('func', []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array('count', [[]]); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array('A::func', []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array('A::static_func', []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array(['A', 'func'], []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array(['A', 'static_func'], []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array([new A, 'func'], []); } catch (Exception $e) { wrap($e); }
    try { forward_static_call_array([new A, 'static_func'], []); } catch (Exception $e) { wrap($e); }
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
  }
}

async function positive_tests() {
  try { $x = 'func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'count'; $x([]); } catch (Exception $e) { wrap($e); }
  try { $x = 'async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'C::foobar'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'HH\Vector::fromItems'; $x([]); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['C', 'foobar']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = ['HH\Vector', 'fromItems']; $x([]); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new C, 'foobar']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new Vector, 'firstValue']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = [new Vector, 'fromItems']; $x([]); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; $x::func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; $x::static_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; await $x::async_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'A'; await $x::static_async_func(); } catch (Exception $e) { wrap($e); }
  try { $x = 'C'; $x::foobar(); } catch (Exception $e) { wrap($e); }
  try { $x = 'HH\Vector'; $x::fromItems([]); } catch (Exception $e) { wrap($e); }
  try { $x = 'func'; A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'static_func'; A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'static_async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'foobar'; C::$x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'fromItems'; Vector::$x([]); } catch (Exception $e) { wrap($e); }

  A::positive_test1();
  C::positive_test1();

  try { $x = 'A'; new $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'HH\Vector'; new $x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'func'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'static_func'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'async_func'; await $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'static_async_func';  await $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new C; $x = 'foobar'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new Vector; $x = 'firstValue'; $obj->$x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new Vector; $x = 'fromItems'; $obj->$x([]); } catch (Exception $e) { wrap($e); }
  try { $obj = null; $obj?->foobar(); } catch (Exception $e) { wrap($e); }

  try { array_map('func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('count', [[]]); } catch (Exception $e) { wrap($e); }
  try { array_map('A::func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('A::static_func', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('C::foobar', [true]); } catch (Exception $e) { wrap($e); }
  try { array_map('HH\Vector::fromItems', [[]]); } catch (Exception $e) { wrap($e); }
  try { array_map(['A', 'func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['A', 'static_func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['C', 'foobar'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map(['HH\Vector', 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }
  try { array_map([new A, 'func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new A, 'static_func'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new C, 'foobar'], [true]); } catch (Exception $e) { wrap($e); }
  try { array_map([new Vector, 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }

  $x = Vector::fromItems([[]]);
  try { $x->map('func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('count'); } catch (Exception $e) { wrap($e); }
  try { $x->map('A::func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('A::static_func'); } catch (Exception $e) { wrap($e); }
  try { $x->map('C::foobar'); } catch (Exception $e) { wrap($e); }
  try { $x->map('HH\Vector::fromItems'); } catch (Exception $e) { wrap($e); }
  try { $x->map(['A', 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
  try { $x->map(['HH\Vector', 'fromItems']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new C, 'foobar']); } catch (Exception $e) { wrap($e); }
  try { $x->map([new Vector, 'fromItems']); } catch (Exception $e) { wrap($e); }

  try { call_user_func('func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('count', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func('A::func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('A::static_func'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('C::foobar'); } catch (Exception $e) { wrap($e); }
  try { call_user_func('HH\Vector::fromItems', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['A', 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(['HH\Vector', 'fromItems'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new C, 'foobar']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new Vector, 'firstValue']); } catch (Exception $e) { wrap($e); }
  try { call_user_func([new Vector, 'fromItems'], []); } catch (Exception $e) { wrap($e); }

  try { call_user_func_array('func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('count', [[]]); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('HH\Vector::fromItems', [[]]); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('A::func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array('A::static_func', []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(['HH\Vector', 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(['A', 'func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(['A', 'static_func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new A, 'func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new A, 'static_func'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new Vector, 'firstValue'], []); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array([new Vector, 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }

  A::positive_test2();
  C::positive_test2();

  try { fb_call_user_func_safe('func'); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('count', []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('A::func'); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('A::static_func'); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('C::foobar'); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('HH\Vector::fromItems', []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe(['A', 'func']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe(['A', 'static_func']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe(['C', 'foobar']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe(['HH\Vector', 'fromItems'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe([new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe([new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe([new C, 'foobar']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe([new Vector, 'firstValue']); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe([new Vector, 'fromItems'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe('foobar'); } catch (Exception $e) { wrap($e); }

  try { fb_call_user_func_safe_return('func', false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('count', false, []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('A::func', false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('A::static_func', false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('C::foobar', false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('HH\Vector::fromItems', false, []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return(['A', 'func'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return(['A', 'static_func'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return(['C', 'foobar'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return(['HH\Vector', 'fromItems'], false, []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return([new A, 'func'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return([new A, 'static_func'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return([new C, 'foobar'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return([new Vector, 'firstValue'], false); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return([new Vector, 'fromItems'], false, []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_safe_return('foobar', false); } catch (Exception $e) { wrap($e); }

  try { fb_call_user_func_array_safe('func', []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe('count', [[]]); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe('A::func', []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe('A::static_func', []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe('HH\Vector::fromItems', [[]]); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe(['A', 'func'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe(['A', 'static_func'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe(['HH\Vector', 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe([new A, 'func'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe([new A, 'static_func'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe([new Vector, 'firstValue'], []); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe([new Vector, 'fromItems'], [[]]); } catch (Exception $e) { wrap($e); }
  try { fb_call_user_func_array_safe('foobar', []); } catch (Exception $e) { wrap($e); }

  try { $x = 'cmp'; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::cmp'; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'A::static_cmp'; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = 'CCmp::foobar'; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'cmp']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['A', 'static_cmp']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = ['CCmp', 'foobar']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'cmp']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new A, 'static_cmp']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = [new CCmp, 'foobar']; $y = [2, 1]; usort($y, $x); } catch (Exception $e) { wrap($e); }
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

  fb_call_user_func_safe($k ==> {}, true);
  fb_call_user_func_safe_return($k ==> {}, false, true);
  fb_call_user_func_array_safe($k ==> {}, [true]);
  fb_call_user_func_safe(new Invokable, true);
  fb_call_user_func_safe_return(new Invokable, false, true);
  fb_call_user_func_array_safe(new Invokable, [true]);

  $x = [2, 1];
  usort($x, ($k1, $k2) ==> { return $k1 <=> $k2; });

  $x = [2, 1];
  usort($x, new InvokableCmp);
}

echo "=============== positive tests =====================\n";
HH\Asio\join(positive_tests());

echo "=============== negative tests =====================\n";
HH\Asio\join(negative_tests());

set_error_handler(
  function ($type, $msg, $file) {
    if ($file !== "") throw new Exception($msg);
  }
);
echo "=============== positive tests (exceptions) ========\n";
HH\Asio\join(positive_tests());
