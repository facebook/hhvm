<?hh // strict

namespace NS_async_functions2;

async function f(): Awaitable<int> {

  echo 'Inside ' . __FUNCTION__ . "\n";

  // ...

  echo "Enter await\n";
  $r1 = await g();	// $r1 = int($r1); that is, the int is unwrapped from the Awaitable object
  echo "Exit await; \$r1 = ";
  var_dump($r1);

  return $r1;		// int($r1) is wrapped by an Awaitable object, which is returned
}

async function g(): Awaitable<int> {

  echo 'Inside ' . __FUNCTION__ . "\n";

  // ...
  $r2 = 10;

  return $r2;		// int($r2) is wrapped by an Awaitable object, which is returned
}

function main (): void {
  $v = f();		// $v = object(HH\StaticWaitHandle)#1 (0)
  echo "\$v = ";
  var_dump($v);
}

/* HH_FIXME[1002] call to main in strict*/
main();
