<?hh // strict

namespace NS_as1;

// async and abstract methods ---------------------------------

interface I1 {
  public function f1(): Awaitable<string>;	// async not allowed here
}

class C1 implements I1 {
  public async function f1(): Awaitable<string> { return "abc"; }
}

// test basic async functions ---------------------------------

async function calc(): Awaitable<int> {
  await \HH\Asio\usleep(2000000);
  return 100;
}

async function control(): Awaitable<void> {
  $awaitable = calc();
  $result = await $awaitable;
//  $result = await calc();	// short version of the 2 assignments above

  await calc();

  echo "\$result = $result\n";
}

// define a pair of functions to be awaited on -----------------

async function dbq1(): Awaitable<int> {
  await \HH\Asio\usleep(3000000);
  return 3000;
}

async function dbq2(): Awaitable<int> {
  $v = \HH\Asio\usleep(1000000);
  return 4000;
}

// some async iterators

async function countdown1(int $start): AsyncIterator<int> {
  for ($i = $start; $i >= 0; --$i) {
    await \HH\Asio\usleep(1000000); // Sleep for 1 second
    yield $i;
  }
}

async function use_countdown1(): Awaitable<void> {
  $async_gen = countdown1(3);
  foreach ($async_gen await as $value) {
    // $value is of type int here
    var_dump($value);
  }
}

async function countdown2(int $start): AsyncKeyedIterator<int, string> {
  for ($i = $start; $i >= 0; --$i) {
    await \HH\Asio\usleep(1000000);
    yield $i => (string)$i;
  }
}

async function use_countdown2(): Awaitable<void> {
  foreach (countdown2(3) await as $num => $str) {
    // $num is of type int
    // $str is of type string
    var_dump($num, $str);
  }
}

// async blocks ------------------------

async function doit(): Awaitable<void> {

  await async {		// block result has type awaitable<void>
  };			// implicit return nothing
  var_dump(async {});
  echo "-----------------\n";

  await async {		// block result has type awaitable<void>
    return;		// explicit return nothing
  };
  var_dump(async { return; });
  echo "-----------------\n";

  await async {		// block result has type awaitable<int>
    return 123;
  };

  $v = await async {		// block result has type awaitable<int>
    return 123;
  };
  var_dump($v, async { return 123; });
  echo "-----------------\n";

  $v = await async {		// block result has type awaitable<float>
    return 1.23;
  };
  var_dump($v, async { return 1.23; });
  echo "-----------------\n";

  $v = await \HH\Asio\v(array(async { return 10; }, async { return 1.2; }));
// first block result has type awaitable<int>, the second, awaitable<float>

  var_dump($v);
  echo "-----------------\n";

  $v = await \HH\Asio\v(array(async { return 1.2; }, async { return 10; }));
// first block result has type awaitable<float>, the second, awaitable<int>
  var_dump($v);
}

// return type is a composite of all possible types returned ---------------

async function threeWay(int $p): Awaitable<?arraykey> {
  if ($p == 0) return null;
  elseif ($p < 0) return 10;
  else return "abc";
}

// In what constexts can we use await directly? -----------------------

async function fDirect(): Awaitable<int> {
//**  takesAnInt(await calc());	// runtime Fatal error: syntax error, unexpected T_AWAIT, expecting ')'
//  $a = array(await calc());	// runtime Fatal error: syntax error, unexpected T_AWAIT, expecting ')'
//  $vect = Vector {await calc()};	// runtime Fatal error: syntax error, unexpected T_AWAIT
//  $t2 = tuple(await calc());	// runtime Fatal error: syntax error, unexpected T_AWAIT, expecting ')'
//  $s = shape('x' => await calc());	// runtime Fatal error: syntax error, unexpected T_AWAIT
//  $cl = clone await calc();	// runtime Fatal error: syntax error, unexpected T_AWAIT
//  $v = (await calc()) instanceof C1;	// runtime Fatal error: syntax error, unexpected T_AWAIT
//  $v = async { return 10; } == await calc();	// runtime Fatal error: syntax error, unexpected T_AWAIT
//  $v = true ? (await calc()) : (async { return 5; });	// runtime Fatal error: syntax error, unexpected T_AWAIT

  await calc();			// in an expression statement
  $v = await calc();		// RHS of a simple assignment
//  return (await calc());	// runtime Fatal error: syntax error, unexpected T_AWAIT
  return await calc();		// OK as the full expression in a return
}

function takesAnInt(int $p): void {
}

// the main program ------------------------

async function main(): Awaitable<void> {
  echo "\nTest some basics ======================\n\n";

///*
  await control();

  echo "\nWait on multiple async operations ======================\n\n";

  $awaitables = array(dbq1(), dbq2());
  $groupAwaitables = \HH\Asio\v($awaitables);
  await $groupAwaitables;
  await \HH\Asio\v(array(dbq1(), dbq2()));	// short version of the 3 statements above

  echo "\nUse async iterators ======================\n\n";

  await use_countdown1();
  await use_countdown2();
//*/
  echo "\nTest async blocks ======================\n\n";

  await doit();

  echo "\nCheck usage contexts for await ======================\n\n";

  await fDirect();
}

/* HH_FIXME[1002] call to main in strict*/
\HH\Asio\join(main());
