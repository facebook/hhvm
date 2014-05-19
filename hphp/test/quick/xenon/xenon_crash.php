<?hh

// Testing that we don't crash due to xenon

class X {
  function __destruct() {
    var_dump(__METHOD__);
  }
}

class A {
  function __destruct() {
    var_dump(__METHOD__);
  }

  async function gen1($a) {
    await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
    return $a + 1;
  }

  async function gen2($a) {
    await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
    $x = $this->gen1($a)->join();
    return $x;
  }

  async function genBar($a) {
    $x = new X;
    await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
    return $a + 2;
  }

  async static function genFoo($a) {
    $a++;
    list($x, $y) = await GenArrayWaitHandle::Create(
      array(
        (new A)->genBar($a),
        (new A)->genBar($a + 1),
        (new A)->gen2($a + 2)
      )
    );
    return $x + $y;
  }
}

function main($a) {
  return A::genFoo($a)->join();
}

var_dump(main(42));
