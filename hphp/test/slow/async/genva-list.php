<?hh

async function foo($x, $suspend = false, $throw = false) {
  if ($suspend) await RescheduleWaitHandle::Create(0, 0);
  if ($throw) throw new Exception;
  return $x + 1;
}

async function bar($x) {
  return array($x + 1, array($x + 2, $x + 3));
}

async function herp() {
  list(
    $a,
    $b,
    $c,
    $d,
    list($f1, list($f2, $f3)),
    $e,
    $f,
  ) = await genva(
    foo(1),
    foo(2),
    foo(3),
    foo(4),
    bar(42),
    foo(5),
    foo(6),
  );
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  list(
    $a,
    $b,
    $c,
    $d,
    list($f1, list($f2, $f3)),
    $e,
    $f,
  ) = await genva(
    foo(1),
    foo(2, true),
    foo(3, true),
    foo(4),
    bar(42),
    foo(5, true),
    foo(6),
  );
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  list(
    $a,
    $b,
    $c,
    $d,
    list($f1, list($f2, $f3)),
    $e,
    $f,
  ) = await genva(
    foo(1, true),
    foo(2),
    foo(3),
    foo(4, true),
    bar(42),
    foo(5),
    foo(6, true),
  );
  var_dump($a, $f);
  var_dump($f1, $f3);

  echo "================================\n";

  try {
    await genva(
      foo(1, true),
      foo(2),
      foo(3),
      foo(4, false, true),
      bar(42),
      foo(5),
      foo(6, true),
    );
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  try {
    list(
      $a,
      $b,
      $c,
      ,
      list($f1, list($f2, $f3)),
      $e,
      $f,
    ) = await genva(
      foo(1, true),
      foo(2),
      foo(3),
      foo(4, true, true),
      bar(42),
      foo(5),
      foo(6, true),
    );
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  try {
    list(
      $a,
      $b,
      $c,
      $d,
      list($f1, list($f2, $f3)),
      $e,
      ,
    ) = await genva(
      foo(1, true),
      foo(2),
      foo(3),
      foo(4, true),
      bar(42),
      foo(5),
      foo(6, true, true),
    );
  } catch (Exception $e) { echo "Caught!\n"; }

  echo "================================\n";

  list(
    $a,
    $b,
    $c,
    $d,
    list($f1, list($f2, $f3)),
    $e,
    $f,
  ) = await genva(
    foo(1),
    foo(2),
    foo(3),
    foo(4),
    bar(42),
    foo(5),
    foo(6),
  );
  var_dump($a, $f);
  var_dump($f1, $f3);
}

function derp() {
  HH\Asio\join(herp());
}

derp();
