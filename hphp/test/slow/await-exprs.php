<?hh

<<file:__EnableUnstableFeatures(
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

async function f(int $x): Awaitable<int> {
  echo "start f($x)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end f($x)\n";
  return $x + 1;
}

async function g(int $x): Awaitable<int> {
  echo "start g($x)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end g($x)\n";
  return $x;
}

async function h(?int $x): Awaitable<?int> {
  echo "start h($x)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end h($x)\n";
  return $x;
}

class A {
  function __construct(public $a = null) {}
  function opt(int $i) {
    return $i > 0 ? $this : null;
  }
  async function genOpt(int $i) {
    echo "start genOpt($i)\n";
    await RescheduleWaitHandle::create(0, 0);
    echo "end genOpt($i)\n";
    return $i > 0 ? $this : null;
  }
}

async function two($a, $b) {
  echo "start two($a, $b)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end two($a, $b)\n";
  return $a + $b;
}

async function all(...$args) {
  $list = implode(', ', $args);
  echo "start all($list)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end all($list)\n";
  return HH\Lib\Math\sum($args);
}

async function id($v) {
  echo "start id()\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end id()\n";
  return $v;
}

function io(inout $x) {
  $r = $x;
  $x = 42;
  return $r;
}

<<__EntryPoint>>
async function main() {
  var_dump(await f(1) + await g(2));
  var_dump(await f(await g(1)) + await f(await g(10)));
  var_dump(await f(await f(await g(1)) + await f(await g(2))));
  var_dump(await f(1) + await f(await g(2)));

  if (await f(1) === 2) echo "ok.\n";
  if (await f(1) === 2 || await f(2) === 3) echo "ok.\n";
  if (await f(1) === 0 || await f(2) === 3) echo "ok.\n";
  if (await f(1) === 2 && await f(2) === 3) echo "ok.\n";
  if (await f(1) === 2 && !(await f(2) === 0 || await f(await g(3)) === 0)) echo "ok.\n";
  if (await f(1) === 2 || !(await f(2) === 0 && await f(await g(3)) === 0)) echo "ok.\n";
  if (await f(1) === 0 || !(await f(2) === 0 || await f(await g(3)) === 0)) echo "ok.\n";
  if (await f(1) === 0 || !(await f(2) === 3 && await f(await g(3)) === 0)) echo "ok.\n";

  var_dump(await h(null) ?? await h(1));
  var_dump(await h(1) ?? await h(1));
  var_dump(await h(null) ? await h(1) : await h(2));
  var_dump(await h(1) ? await h(2) : await h(3));
  var_dump(await h(1) ? await h(2) : await h(3));

  var_dump(
    ((await g(1) === 1 || await g(2) === 2) ? 1 : 0) &
    ((await g(await g(10)) && await g(await g(20))) ? 1 : 0)
  );

  $t1 = true;   $t2 = "true";  $t3 = 1;  $t4 = new stdclass;
  $f1 = false;  $f2 = "";      $f3 = 0;  $f4 = null;

  echo "t1 ---\n";
  $t1 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "t2 ---\n";
  $t2 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "t3 ---\n";
  $t3 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "t4 ---\n";
  $t4 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "f1 ---\n";
  $f1 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "f2 ---\n";
  $f2 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "f3 ---\n";
  $f3 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  echo "f4 ---\n";
  $f4 ??= await f(
    await g(await g(1)) + await g(2) + (await g(0) ? await f(10) : await f(20))
  ) + await f(12);

  var_dump($t1, $t2, $t3, $t4, $f1, $f2, $f3, $f4);

  $lval = vec[dict['a' => vec[10, 20, 30]], vec[]];

  echo "l1 ---\n";
  $lval[await f(0)][] = 1;

  echo "l2 ---\n";
  $lval[await f(0)][] = await g(1);

  echo "l3 ---\n";
  $lval[1][] = await g(2);

  echo "l4 ---\n";
  $lval[await g(0) ? await f(10) : await f(0)][await g(0)] = await h(null);

  echo "l5 ---\n";
  $lval[await g(0) ? await f(10) : await f(0)][await g(0)] ??=
    $lval[await g(0)]['x'][await f(0) + await g(1)] ?? $lval[await g(0)]['a'][await f(0) + await g(1)];

  echo "l6 ---\n";
  $lval[await g(0) ? await f(10) : await f(0)][await g(1)] =
    $lval[await g(0)]['x'][await f(0) + await g(1)] ?? $lval[await g(0)]['a'][await f(0) + await g(1)] + 5;

  echo "l7 ---\n";
  $lval[await g(0) ? await f(10) : await f(0)][await g(0)] ??=
    $lval[await g(0)]['x'][await f(0) + await g(1)] ?? $lval[await g(0)]['a'][await f(0) + await g(1)] + 1000;

  var_dump($lval);


  // list(..) = ..
  list($x, $y, $z) = vec[await f(0), await g(await g(1)), await f(1)];
  var_dump($x, $y, $z);

  $lv = dict[0 => vec[null, null], 1 => dict[2 => null, 10 => null], 3 => null];
  $rv = vec['x', 'y', 'z', 'q', 'r'];
  list($lv[await g(0)][await g(await g (1))],
       $lv[await f(0) + await g(0)][await g(await g(1)) + await f(0)],
       $lv[__hhvm_intrinsics\launder_value(0)][await g(await g(0))],
       $lv[await f(0)][__hhvm_intrinsics\launder_value(10)],
       $lv[__hhvm_intrinsics\launder_value(3)]) = $rv;
  var_dump($lv);

  $lv = dict[0 => vec[null, null], 1 => dict[2 => null, 10 => null], 3 => null];
  $x = dict[1 => vec[dict[10 => $rv]]];
  list($lv[await g(0)][await g(await g (1))],
       $lv[await f(0) + await g(0)][await g(await g(1)) + await f(0)],
       $lv[__hhvm_intrinsics\launder_value(0)][await g(await g(0))],
       $lv[await f(0)][__hhvm_intrinsics\launder_value(10)],
       $lv[__hhvm_intrinsics\launder_value(3)]) = $x[await g(1)][__hhvm_intrinsics\launder_value(0)][await f(await g(await g(5)) + await g(4))];
  var_dump($lv);

  $m = vec[null, null, null, null];
  $v = vec[vec[1, 2], vec['a', 'b']];
  list(
    list(
      $m[await(g(await g(0)))],
      $m[await(g(await g(1)))],
    ),
    list(
      $m[await(g(await g(2)))],
      $m[await(g(await g(3)))],
    ),
  ) = $v;
  var_dump($m);

  // $x?->...() (no short-circuiting of args)
  $v1 = new A(new A(new A(new A(await g(42)))))
    ?->opt(await g(3))
    ?->opt(await g(2))
    ?->opt(await g(1))
    ?->opt(await g(1))
    ?->opt(await g(0))
    ?->opt(await g(1000))
    ?->opt(await g(2000));
  var_dump($v1);

  $v1 = new A(new A(new A(new A(42))))
    |> await $$?->genOpt(await g(3))
    |> await $$?->genOpt(await g(2))
    |> await $$?->genOpt(await g(1))
    |> await $$?->genOpt(await g(0))
    |> await $$?->genOpt(await g(1000))
    |> await $$?->genOpt(await g(2000));
  var_dump($v1);

  // $x?->...
  $a = new A(vec[new A, new A(vec['x', new A]), vec[new A(1)]]);
  var_dump($a?->a[await g(1)]?->a[await g(0)]);
  var_dump($a?->a[await g(1)]?->a[await g(1)]?->a);
  var_dump($a?->a[await g(1)]?->a[await g(1)]?->a?->a[await g(1000)]);
  var_dump($a?->a[await g(0)]?->a?->a[await g(1000)]);

  // |?> (does short-circuit)
  $v1 = new A(new A(new A(new A(42))))
    |?> await $$->genOpt(await g(3))
    |?> await $$->genOpt(await g(2))
    |?> await $$->genOpt(await g(1))
    |?> await $$->genOpt(await g(0))
    |?> await $$->genOpt(await g(9000))
    |?> await $$->genOpt(await g(9900));
  var_dump($v1);

  $v1 = new A(new A(new A(new A(42))))
    ?->opt(await g(3))
    |?> await $$->genOpt(await g(2))
    |?> ((await $$->genOpt(await g(1)))?->opt(await g(1)));
  var_dump($v1);

  $v1 = new A(new A(new A(new A(42))))
    ?->opt(await g(3))
    |?> await $$->genOpt(await g(2))
    |?> ((await $$->genOpt(await g(0)))?->opt(await g(1000)));
  var_dump($v1);

  $v1 = new A(new A(new A(new A(42))))
    |> ((await $$->genOpt(await g(1)))->opt(await g(2)));
  var_dump($v1);

  $v1 = new A(new A(new A(new A(42))))
    ?->opt(await g(3))
    |?> await $$->genOpt(await g(2))
    |?> ((await $$->genOpt(await g(0)))?->opt(await g(1000)))
    |?> $$->opt(await g(9000));
  var_dump($v1);
  // <?

  // ++.., ..++, --.., ..--
  $v = vec[1, 2, 3, vec[4, 5]];
  $v[await g(1)]++;
  ++$v[await g(3)][await g(1)];
  var_dump($v);

  // f(.., ...await ..)
  $v = vec[vec[1, 2, 3, 4, 5, 6], vec[1, 2]];
  var_dump(await two(...(await id($v))[1]));
  var_dump(await all(...$v[await g(0)]));
  var_dump(await all(await g(7), await g(8), ...(await id($v))[await g(0)]));
  var_dump(await all(await g(7), await g(8), ...await id($v[await g(0)])));

  // f(inout $x[await $y])
  $v = vec[dict[10 => 20], 2];
  io(inout $v[await g(1)]);
  io(inout $v[await g(0)][await g(10)]);
  var_dump($v);

  // while (..) { }
  $x = 5;
  while ((await g($x) < await g($x + 1) && 0 !== await g($x + 2)) || await g(0) !== 0) {
    if (await g(10 + $x) === await g(10) || await g(10 + $x) === 10 + $x) {
      var_dump($x);
    }
    $x -= 1;
  }
  var_dump($x);

  $x = 5;
  while (await id($x !== 0)) {
    if (await g(10 + $x) === await g(10) || await g(10 + $x) === 10 + $x) {
      var_dump($x);
    }
    $x -= 1;
  }
  var_dump($x);

  $x = 5;
  while (await g($x) < await g(8)) {
    $x = await g(await g($x) + await g(0)) + await g(1);
    var_dump($x);
  }
  var_dump($x);

  // do { } while (..)
  $x = 5;
  do {
    if (await g(10 + $x) === await g(10) || await g(10 + $x) === 10 + $x) {
      var_dump($x);
    }
    $x -= 1;
  } while ((await g($x) < await g($x + 1) && 0 !== await g($x + 2)) || await g(0) !== 0);
  var_dump($x);

  $x = 5;
  do {
    if (await g(10 + $x) === await g(10) || await g(10 + $x) === 10 + $x) {
      var_dump($x);
    }
    $x -= 1;
  } while (await id($x !== 0));
  var_dump($x);

  // for (; ..; ..) { }
  for (
    $a = 1,
    $b = await g(2),
    $c = await g(3) + await g(await g(4)),
    $d = await g(5) == 6 || await g(6) == 6 || await g(7) == 6
    ;
    await g($b) !== await g($c) && await g($a) !== await g($c) && await id($d)
    ;
    $b *= await g(2) + await g(0),
    $c = (await g($c) === $c ? await g($c) : await g(10000)),
    $d = await id($b < 10)
  ) {
    var_dump($a, $b, $c, $d);
    $d = await id($d) ? await g(await g(0)) === await g(0) : await id(false);
  }
  var_dump($a, $b, $c, $d);

  // These are parse errors:
  //var_dump($v[await g(2)]--);
  //var_dump(++$v[await g(3)][await g(1)]);
}
