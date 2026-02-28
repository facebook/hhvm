<?hh

async function foo($x, $suspend = false, $throw = false) :Awaitable<mixed>{
  if ($suspend) await RescheduleWaitHandle::create(0, 0);
  if ($throw) throw new Exception;
  return $x + 1;
}

async function bar($x) :Awaitable<mixed>{
  return vec[$x + 1, vec[$x + 2, $x + 3]];
}

async function herp() :Awaitable<mixed>{
  concurrent {
    $a = await foo(1);
    $b = await foo(2);
    $c = await foo(3);
    $d = await foo(4);
    list($f1, list($f2, $f3)) = await bar(42);
    $e = await foo(5);
    $f = await foo(6);
  }
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  concurrent {
    $a = await foo(1);
    $b = await foo(2, true);
    $c = await foo(3, true);
    $d = await foo(4);
    list($f1, list($f2, $f3)) = await bar(42);
    $e = await foo(5, true);
    $f = await foo(6);
  }
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  concurrent {
    $a = await foo(1, true);
    $b = await foo(2);
    $c = await foo(3);
    $d = await foo(4, true);
    list($f1, list($f2, $f3)) = await bar(42);
    $e = await foo(5);
    $f = await foo(6, true);
  }
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  try {
    concurrent {
      await foo(1, true);
      await foo(2);
      await foo(3);
      await foo(4, false, true);
      await bar(42);
      await foo(5);
      await foo(6, true);
    }
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  try {
    concurrent {
      $a = await foo(1, true);
      $b = await foo(2);
      $c = await foo(3);
      await foo(4, true, true);
      list($f1, list($f2, $f3)) = await bar(42);
      $e = await foo(5);
      $f = await foo(6, true);
    }
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  try {
    concurrent {
      $a = await foo(1, true);
      $b = await foo(2);
      $c = await foo(3);
      $d = await foo(4, true);
      list($f1, list($f2, $f3)) = await bar(42);
      $e = await foo(5);
      await foo(6, true, true);
    }
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  concurrent {
    $a = await foo(1);
    $b = await foo(2);
    $c = await foo(3);
    $d = await foo(4);
    list($f1, list($f2, $f3)) = await bar(42);
    $e = await foo(5);
    $f = await foo(6);
  }
  var_dump($a, $f);
  var_dump($f1, $f3);
}

function derp() :mixed{
  HH\Asio\join(herp());
}


<<__EntryPoint>>
function main_genva_list() :mixed{
derp();
}
