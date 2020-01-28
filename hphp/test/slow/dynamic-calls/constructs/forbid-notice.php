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

}

class C extends D {
  public function __call($a, $b) {}














}

class G  {
  <<__DynamicallyCallable>> public function __call($a, $b) {}

}

class E extends G {
  <<__DynamicallyCallable>> public function __call($a, $b) {}








}

class CCmp {
  public function __call($a, $b) { return $b[0] <=> $b[1]; }

}

class CCmp2 {
  <<__DynamicallyCallable>> public function __call($a, $b) { return $b[0] <=> $b[1]; }

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





    $x = 'static_func';
    try { self::$x(); } catch (Exception $e) { wrap($e); }
    try { static::$x(); } catch (Exception $e) { wrap($e); }
    try { parent::$x(); } catch (Exception $e) { wrap($e); }






    $x = 'static_async_func';
    try { await self::$x(); } catch (Exception $e) { wrap($e); }
    try { await static::$x(); } catch (Exception $e) { wrap($e); }
    try { await parent::$x(); } catch (Exception $e) { wrap($e); }
  }

  public static async function negative_test1() {




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

async function positive_tests() {
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
  try { $x = varray[new C, 'foobar']; $x(); } catch (Exception $e) { wrap($e); }

  try { $x = 'A'; $x::static_func(); } catch (Exception $e) { wrap($e); }

  try { $x = 'A'; await $x::static_async_func(); } catch (Exception $e) { wrap($e); }


  try { $x = 'static_func'; A::$x(); } catch (Exception $e) { wrap($e); }

  try { $x = 'static_async_func'; await A::$x(); } catch (Exception $e) { wrap($e); }


  A::positive_test1();


  try { $x = 'A'; new $x(); } catch (Exception $e) { wrap($e); }
  try { $obj = new A; $x = 'func'; $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { $obj = new A; $x = 'async_func'; await $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { $obj = new C; $x = 'foobar'; $obj->$x(); } catch (Exception $e) { wrap($e); }

  try { array_map('func', varray[true]); } catch (Exception $e) { wrap($e); }
  //try { array_map('A::func', varray[true]); } catch (Exception $e) { wrap($e); } // fatal
  try { array_map('A::static_func', varray[true]); } catch (Exception $e) { wrap($e); }

  //try { array_map(varray['A', 'func'], varray[true]); } catch (Exception $e) { wrap($e); } // fatal
  try { array_map(varray['A', 'static_func'], varray[true]); } catch (Exception $e) { wrap($e); }

  try { array_map(varray[new A, 'func'], varray[true]); } catch (Exception $e) { wrap($e); }
  try { array_map(varray[new A, 'static_func'], varray[true]); } catch (Exception $e) { wrap($e); }
  try { array_map(varray[new C, 'foobar'], varray[true]); } catch (Exception $e) { wrap($e); }

  $x = Vector::fromItems(varray[varray[]]);
  try { $x->map('func'); } catch (Exception $e) { wrap($e); }

  try { $x->map('A::static_func'); } catch (Exception $e) { wrap($e); }


  try { $x->map(varray['A', 'static_func']); } catch (Exception $e) { wrap($e); }

  try { $x->map(varray[new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(varray[new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { $x->map(varray[new C, 'foobar']); } catch (Exception $e) { wrap($e); }

  try { call_user_func('func'); } catch (Exception $e) { wrap($e); }

  try { call_user_func('A::static_func'); } catch (Exception $e) { wrap($e); }


  try { call_user_func(varray['A', 'static_func']); } catch (Exception $e) { wrap($e); }

  try { call_user_func(varray[new A, 'func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(varray[new A, 'static_func']); } catch (Exception $e) { wrap($e); }
  try { call_user_func(varray[new C, 'foobar']); } catch (Exception $e) { wrap($e); }

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
  try { $x = varray[new CCmp, 'foobar']; $y = varray[2, 1]; usort(inout $y, $x); } catch (Exception $e) { wrap($e); }
}

async function negative_tests() {
  func();
  count(varray[]);
  await async_func();

  A::static_func();

  await A::static_async_func();

  Vector::fromItems(varray[]);

  $x = ($k ==> {});
  $x(1);

  A::negative_test1();

  HH\class_meth(A::class, 'static_func')();

  new A;
  new Vector;

  $obj = new A;
  $obj->func();

  await $obj->async_func();


  $obj = new C;
  $obj->foobar();

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
  $x = varray[new E, 'foobar']; $x();

  $x = 'A'; $x::static_func2();

  $x = 'A'; await $x::static_async_func2();


  $x = 'static_func2'; A::$x();

  $x = 'static_async_func2'; await A::$x();


  $x = 'F'; new $x();
  $obj = new A; $x = 'func2'; $obj->$x();

  $obj = new A; $x = 'async_func2'; await $obj->$x();

  $obj = new E; $x = 'foobar'; $obj->$x();

  array_map('func2', varray[true]);
  //array_map('A::func2', varray[true]); // fatal
  array_map('A::static_func2', varray[true]);

  //array_map(varray['A', 'func2'], varray[true]); // fatal
  array_map(varray['A', 'static_func2'], varray[true]);

  array_map(varray[new A, 'func2'], varray[true]);
  array_map(varray[new A, 'static_func2'], varray[true]);
  array_map(varray[new E, 'foobar'], varray[true]);

  $x = Vector::fromItems(varray[varray[]]);
  $x->map('func2');

  $x->map('A::static_func2');


  $x->map(varray['A', 'static_func2']);

  $x->map(varray[new A, 'func2']);
  $x->map(varray[new A, 'static_func2']);
  $x->map(varray[new E, 'foobar']);

  call_user_func('func2');

  call_user_func('A::static_func2');


  call_user_func(varray['A', 'static_func2']);

  call_user_func(varray[new A, 'func2']);
  call_user_func(varray[new A, 'static_func2']);
  call_user_func(varray[new E, 'foobar']);

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
  $x = varray[new CCmp2, 'foobar']; $y = varray[2, 1]; usort(inout $y, $x);
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
