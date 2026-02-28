<?hh

// Testing that we don't crash due to xenon

async function genList(...$args) :Awaitable<mixed>{
  await AwaitAllWaitHandle::fromVec($args);
  return array_map($wh ==> \HH\Asio\result($wh), $args);
}

class A {
  async function gen1($a) :Awaitable<mixed>{
    await RescheduleWaitHandle::create(0, 0); // simulate blocking I/O
    return $a + 1;
  }

  async function gen2($a) :Awaitable<mixed>{
    await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
    $x = HH\Asio\join($this->gen1($a));
    return $x;
  }

  async function genBar($a) :Awaitable<mixed>{
    await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
    return $a + 2;
  }

  static async function genFoo($a) :Awaitable<mixed>{
    $a++;
    list($x, $y) = await genList(
      (new A)->genBar($a),
      (new A)->genBar($a + 1),
      (new A)->gen2($a + 2),
    );
    return $x + $y;
  }
}

function main($a) :mixed{
  return HH\Asio\join(A::genFoo($a));
}


<<__EntryPoint>>
function main_xenon_crash() :mixed{
var_dump(main(42));
}
