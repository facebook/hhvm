<?hh

<<__Rx>>        function rx_caller($c)[rx]      { return $c(); }
<<__RxShallow>> function shallow_caller($c)[rx_shallow] { return $c(); }

function check_rx_level($name, $c) {
  echo $name;
  try {
    rx_caller($c);
    echo " is Rx\n";
    return;
  } catch (BadMethodCallException $_) {}
  try {
    shallow_caller($c);
    echo " is RxShallow or RxLocal\n";
    return;
  } catch (BadMethodCallException $_) {}
  echo " is NonRx\n";
}


function create_php_anon_function()[rx] {
  return function () { return 1; };
}

function create_php_closure()[rx] {
  $x = 1;
  return function () use ($x) { return $x; };
}

function create_short_lambda()[rx] {
  $x = 1;
  return () ==> { return $x; };
}

function test_closure_types() {
  check_rx_level("php anonymous function", create_php_anon_function());
  check_rx_level("php closure", create_php_closure());
  check_rx_level("short lambda", create_short_lambda());
}


async function check_async_block()[rx] {
  // we can't defer invoking an async block so we have to test it differently
  return await async { return 1; };
}

async function test_async_block() {
  $x = await check_async_block();
  invariant($x === 1, "");
  echo "async block works in Rx function\n";
}


class C {
  public static function create_closure_in_static_method()[rx] {
    return () ==> { return 1; };
  }

  public function create_closure_in_instance_method()[rx] {
    return () ==> { return 1; };
  }
}

function test_closure_from_method() {
  check_rx_level(
    "closure from static method",
    C::create_closure_in_static_method()
  );
  check_rx_level(
    "closure from instance method",
    (new C())->create_closure_in_instance_method()
  );
}


function create_nested_closure()[rx] {
  $x = 1;
  $c = () ==> {
    return () ==> { return $x; };
  };
  return $c();
}

function create_nested_closure_with_override()[rx] {
  $x = 1;
  $c = () ==> {
    return <<__RxShallow>> () ==> { return $x; };
  };
  return $c();
}

function create_nested_closure_with_override_to_nonrx()[rx] {
  $x = 1;
  $c = () ==> {
    return <<__NonRx>> () ==> { return $x; };
  };
  return $c();
}

function create_nested_closure_with_override_to_nonrx_and_more_nesting()[rx] {
  $x = 1;
  $c = () ==> {
    return <<__NonRx>> () ==> {
      return () ==> { return $x; };
    };
  };
  return $c();
}

function test_nested() {
  check_rx_level("nested closure", create_nested_closure());
  check_rx_level(
    "nested closure with override",
    create_nested_closure_with_override()
  );
  check_rx_level(
    "nested closure with override to NonRx",
    create_nested_closure_with_override_to_nonrx()
  );
  $c = create_nested_closure_with_override_to_nonrx_and_more_nesting();
  check_rx_level(
    "nested closure with override to NonRx and more nesting",
    $c()
  );
}


<<__EntryPoint>>
async function main() {
  test_closure_types();
  await test_async_block();
  test_closure_from_method();
  test_nested();
}
