<?php

// Testing that we don't crash due to xenon

async function genList(...$args) {
  await AwaitAllWaitHandle::fromArray($args);
  return array_map($wh ==> \HH\Asio\result($wh), $args);
}

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
    await RescheduleWaitHandle::create(0, 0); // simulate blocking I/O
    return $a + 1;
  }

  async function gen2($a) {
    await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
    $x = HH\Asio\join($this->gen1($a));
    return $x;
  }

  async function genBar($a) {
    $x = new X;
    await RescheduleWaitHandle::create(0, $a); // simulate blocking I/O
    return $a + 2;
  }

  static async function genFoo($a) {
    $a++;
    list($x, $y) = await genList(
      (new A)->genBar($a),
      (new A)->genBar($a + 1),
      (new A)->gen2($a + 2),
    );
    return $x + $y;
  }
}

function main($a) {
  return HH\Asio\join(A::genFoo($a));
}

var_dump(main(42));
