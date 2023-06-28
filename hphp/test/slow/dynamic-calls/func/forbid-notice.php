<?hh

function wrap($e) :mixed{ echo "Exception: {$e->getMessage()}\n"; }

function func() :mixed{}
async function async_func() :Awaitable<mixed>{ return 5; }
function cmp($x, $y) :mixed{ return $x <=> $y; }

<<__DynamicallyCallable>> function func2() :mixed{}
<<__DynamicallyCallable>> async function async_func2() :Awaitable<mixed>{ return 5; }
<<__DynamicallyCallable>> function cmp2($x, $y) :mixed{ return $x <=> $y; }

class Invokable {
  public function __invoke() :mixed{}
}

class InvokableCmp {
  public function __invoke($a, $b) :mixed{ return $a <=> $b; }
}

class B {
  public function func() :mixed{}
  public static function static_func() :mixed{}
  public async function async_func() :Awaitable<mixed>{ return 5; }
  public static async function static_async_func() :Awaitable<mixed>{ return 5; }

  <<__DynamicallyCallable>> public function func2() :mixed{}
  <<__DynamicallyCallable>> public static function static_func2() :mixed{}
  <<__DynamicallyCallable>> public async function async_func2() :Awaitable<mixed>{ return 5; }
  <<__DynamicallyCallable>> public static async function static_async_func2() :Awaitable<mixed>{ return 5; }
}

class A extends B {
  public function __construct()[] {}

  public function func() :mixed{}
  public async function async_func() :Awaitable<mixed>{ return 5; }
  public function cmp($x, $y) :mixed{ return $x <=> $y; }
  public static function static_func() :mixed{}
  public static async function static_async_func() :Awaitable<mixed>{ return 5; }
  public static function static_cmp($x, $y) :mixed{ return $x <=> $y; }

  <<__DynamicallyCallable>> public function func2() :mixed{}
  <<__DynamicallyCallable>> public async function async_func2() :Awaitable<mixed>{ return 5; }
  <<__DynamicallyCallable>> public function cmp2($x, $y) :mixed{ return $x <=> $y; }
  <<__DynamicallyCallable>> public static function static_func2() :mixed{}
  <<__DynamicallyCallable>> public static async function static_async_func2() :Awaitable<mixed>{ return 5; }
  <<__DynamicallyCallable>> public static function static_cmp2($x, $y) :mixed{ return $x <=> $y; }

  public static async function positive_test1() :Awaitable<mixed>{





    $x = 'static_func';
    try { self::$x(); } catch (Exception $e) { wrap($e); }
    try { static::$x(); } catch (Exception $e) { wrap($e); }
    try { parent::$x(); } catch (Exception $e) { wrap($e); }






    $x = 'static_async_func';
    try { await self::$x(); } catch (Exception $e) { wrap($e); }
    try { await static::$x(); } catch (Exception $e) { wrap($e); }
    try { await parent::$x(); } catch (Exception $e) { wrap($e); }
  }

  public static async function negative_test1() :Awaitable<mixed>{




    self::static_func();
    static::static_func();
    parent::static_func();





    await self::static_async_func();
    await static::static_async_func();
    await parent::static_async_func();

    new self;
    new static;
    new parent;






    $x = 'static_func2';
    self::$x();
    static::$x();
    parent::$x();






    $x = 'static_async_func2';
    await self::$x();
    await static::$x();
    await parent::$x();

  }
}

<<__DynamicallyConstructible>>
class F extends A {
}

async function positive_tests() :Awaitable<mixed>{
  try { $x = 'func'; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = 'async_func'; await $x(); } catch (Exception $e) { wrap($e); }
  //try { $x = 'A::func'; $x(); } catch (Exception $e) { wrap($e); } // fatal
  try { $x = 'A::static_func'; $x(); } catch (Exception $e) { wrap($e); }
  //try { $x = 'A::async_func'; await $x(); } catch (Exception $e) { wrap($e); } // fatal
  try { $x = 'A::static_async_func'; await $x(); } catch (Exception $e) { wrap($e); }

  //try { $x = varray['A', 'func']; $x(); } catch (Exception $e) { wrap($e); } // fatal
  try { $x = varray['A', 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  //try { $x = varray['A', 'async_func']; await $x(); } catch (Exception $e) { wrap($e); } // fatal
  try { $x = varray['A', 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }

  try { $x = varray[new A, 'func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = varray[new A, 'static_func']; $x(); } catch (Exception $e) { wrap($e); }
  try { $x = varray[new A, 'async_func']; await $x(); } catch (Exception $e) { wrap($e); }
  try { $x = varray[new A, 'static_async_func']; await $x(); } catch (Exception $e) { wrap($e); }

  try { $x = 'A'; $x::static_func(); } catch (Exception $e) { wrap($e); }

  try { $x = 'A'; await $x::static_async_func(); } catch (Exception $e) { wrap($e); }


  try { $x = 'static_func'; A::$x(); } catch (Exception $e) { wrap($e); }

  try { $x = 'static_async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }


  A::positive_test1();


  try { $x = 'A'; new $x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'func'; $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { $obj = new A; $x = 'async_func'; await $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { array_map('func', varray[true]); } catch (Exception $e) { wrap($e); }
  //try { array_map('A::func', varray[true]); } catch (Exception $e) { wrap($e); } // fatal
  try { array_map('A::static_func', varray[true]); } catch (Exception $e) { wrap($e); }

  //try { array_map(varray['A', 'func'], varray[true]); } catch (Exception $e) { wrap($e); } // fatal
  try { array_map(varray['A', 'static_func'], varray[true]); } catch (Exception $e) { wrap($e); }

  try { array_map(varray[new A, 'func'], varray[true]); } catch (Exception $e) { wrap($e); }
  try { array_map(varray[new A, 'static_func'], varray[true]); } catch (Exception $e) { wrap($e); }

  $x = Vector::fromItems(varray[varray[]]);
  try { $x->map('func'); } catch (Exception $e) { wrap($e); }

  try { $x->map('A::static_func'); } catch (Exception $e) { wrap($e); }


  try { $x->map(varray['A', 'static_func']); } catch (Exception $e) { wrap($e); }

  try { $x->map(varray[new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(varray[new A, 'static_func']); } catch (Exception $e) { wrap($e); }

  try { call_user_func('func'); } catch (Exception $e) { wrap($e); }

  try { call_user_func('A::static_func'); } catch (Exception $e) { wrap($e); }


  try { call_user_func(varray['A', 'static_func']); } catch (Exception $e) { wrap($e); }

  try { call_user_func(varray[new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(varray[new A, 'static_func']); } catch (Exception $e) { wrap($e); }

  try { call_user_func_array('func', varray[]); } catch (Exception $e) { wrap($e); }

  try { call_user_func_array('A::static_func', varray[]); } catch (Exception $e) { wrap($e); }

  try { call_user_func_array(varray['A', 'static_func'], varray[]); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(varray[new A, 'func'], varray[]); } catch (Exception $e) { wrap($e); }
  try { call_user_func_array(varray[new A, 'static_func'], varray[]); } catch (Exception $e) { wrap($e); }

  try { $x = 'cmp'; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }

  try { $x = 'A::static_cmp'; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }


  try { $x = varray['A', 'static_cmp']; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }

  try { $x = varray[new A, 'cmp']; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }
  try { $x = varray[new A, 'static_cmp']; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }
}

async function negative_tests() :Awaitable<mixed>{
  func();
  count(varray[]);
  await async_func();

  A::static_func();

  await A::static_async_func();

  Vector::fromItems(varray[]);

  $x = ($k ==> {});
  $x(1);

  A::negative_test1();

  (A::static_func<>)();

  new A;
  new Vector;

  $obj = new A;
  $obj->func();

  await $obj->async_func();


  $obj = new Vector;
  $obj->firstValue();


  array_map($k ==> {}, varray[true]);
  array_map(new Invokable, varray[true]);

  $x = Vector::fromItems(varray[true]);
  $x->map($k ==> {});
  $x->map(new Invokable);

  $x = varray[2, 1];
  usort(inout $x, ($k1, $k2) ==> { return $k1 <=> $k2; });

  $x = varray[2, 1];
  usort(inout $x, new InvokableCmp);

  $x = 'count'; $x(varray[]);
  array_map('count', varray[varray[]]);
  $x = Vector::fromItems(varray[true]); $x->map('count');
  call_user_func('count', varray[]);
  call_user_func_array('count', varray[varray[]]);

  $x = 'HH\Vector::fromItems'; $x(varray[]);
  $x = varray['HH\Vector', 'fromItems']; $x(varray[]);
  $x = varray[new Vector, 'fromItems']; $x(varray[]);
  $x = 'fromItems'; Vector::$x(varray[]);
  $x = varray[new Vector, 'firstValue']; $x();
  $x = 'HH\Vector'; $x::fromItems(varray[]);
  $obj = new Vector; $x = 'firstValue'; $obj->$x();

  array_map('HH\Vector::fromItems', varray[varray[]]);
  array_map(varray['HH\Vector', 'fromItems'], varray[varray[]]);
  array_map(varray[new Vector, 'fromItems'], varray[varray[]]);
  $x = Vector::fromItems(varray[varray[]]);
  $x->map('HH\Vector::fromItems');
  $x->map(varray['HH\Vector', 'fromItems']);
  $x->map(varray[new Vector, 'fromItems']);
  call_user_func('HH\Vector::fromItems', varray[]);
  call_user_func(varray['HH\Vector', 'fromItems'], varray[]);
  call_user_func(varray[new Vector, 'firstValue']);
  call_user_func(varray[new Vector, 'fromItems'], varray[]);
  call_user_func_array('HH\Vector::fromItems', varray[varray[]]);
  call_user_func_array(varray['HH\Vector', 'fromItems'], varray[varray[]]);
  call_user_func_array(varray[new Vector, 'firstValue'], varray[]);
  call_user_func_array(varray[new Vector, 'fromItems'], varray[varray[]]);

  $x = 'array_map';
  $x('count', varray[]);

  $obj = null; $obj?->foobar();

  idx('foobar', 'key');

  $x = 'func2'; $x();
  $x = 'async_func2'; await $x();
  //$x = 'A::func2'; $x(); // fatal
  $x = 'A::static_func2'; $x();
  //$x = 'A::async_func2'; await $x(); // fatal
  $x = 'A::static_async_func2'; await $x();

  //$x = varray['A', 'func2']; $x(); // fatal
  $x = varray['A', 'static_func2']; $x();
  //$x = varray['A', 'async_func2']; await $x(); // fatal
  $x = varray['A', 'static_async_func2']; await $x();

  $x = varray[new A, 'func2']; $x();
  $x = varray[new A, 'static_func2']; $x();
  $x = varray[new A, 'async_func2']; await $x();
  $x = varray[new A, 'static_async_func2']; await $x();

  $x = 'A'; $x::static_func2();

  $x = 'A'; await $x::static_async_func2();


  $x = 'static_func2'; A::$x();

  $x = 'static_async_func2'; await A::$x();


  $x = 'F'; new $x();
  $obj = new A; $x = 'func2'; $obj->$x();

  $obj = new A; $x = 'async_func2'; await $obj->$x();

  array_map('func2', varray[true]);
  //array_map('A::func2', varray[true]); // fatal
  array_map('A::static_func2', varray[true]);

  //array_map(varray['A', 'func2'], varray[true]); // fatal
  array_map(varray['A', 'static_func2'], varray[true]);

  array_map(varray[new A, 'func2'], varray[true]);
  array_map(varray[new A, 'static_func2'], varray[true]);

  $x = Vector::fromItems(varray[varray[]]);
  $x->map('func2');

  $x->map('A::static_func2');


  $x->map(varray['A', 'static_func2']);

  $x->map(varray[new A, 'func2']);
  $x->map(varray[new A, 'static_func2']);

  call_user_func('func2');

  call_user_func('A::static_func2');


  call_user_func(varray['A', 'static_func2']);

  call_user_func(varray[new A, 'func2']);
  call_user_func(varray[new A, 'static_func2']);

  call_user_func_array('func2', varray[]);

  call_user_func_array('A::static_func2', varray[]);

  call_user_func_array(varray['A', 'static_func2'], varray[]);
  call_user_func_array(varray[new A, 'func2'], varray[]);
  call_user_func_array(varray[new A, 'static_func2'], varray[]);

  $x = 'cmp2'; $y = varray[2, 1]; usort(inout $y, $x);

  $x = 'A::static_cmp2'; $y = varray[2, 1]; usort(inout $y, $x);


  $x = varray['A', 'static_cmp2']; $y = varray[2, 1]; usort(inout $y, $x);

  $x = varray[new A, 'cmp2']; $y = varray[2, 1]; usort(inout $y, $x);
  $x = varray[new A, 'static_cmp2']; $y = varray[2, 1]; usort(inout $y, $x);
}
<<__EntryPoint>>
function main_entry(): void {

  echo "=============== positive tests =====================\n";
  HH\Asio\join(positive_tests());

  echo "=============== negative tests =====================\n";
  HH\Asio\join(negative_tests());

  set_error_handler(
    function ($type, $msg, $file) { throw new Exception($msg); }
  );
  echo "=============== positive tests (exceptions) ========\n";
  HH\Asio\join(positive_tests());
}
