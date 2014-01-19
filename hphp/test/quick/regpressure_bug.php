<?php

/*
 * This test emulates a regpressure case that occurred with inlining
 * (but probably won't occur later as we improve linearscan).
 */

class IPAddressIsh {
  private $binary;
  private $id;

  public function __construct($id) {
    $this->id = $id;
    $this->binary = "asdasd";
  }

  public function __destruct() {
    echo "dtor" . $this->id . "\n";
  }

  public function toBinary() {
    return $this->binary;
  }

  public final function inSubnetWithMask(IPAddressIsh $subnet, string $mask) {
    // Test case for inlining from FB's IPAddress class, removed real
    // logic.
    echo "pre1\n";
    return ($this->toBinary() & $mask) === ($subnet->toBinary() & $mask);
  }

  public function foobar() {
    echo $this->binary;
    echo "\n";
  }
}

function main() {
  $x = new IPAddressIsh('x');
  $y = new IPAddressIsh('y');
  echo $x->inSubnetWithMask($y, "213");
  // Use after and ensure refcounts are still real.
  echo "after1\n";
  $x->foobar();
  echo "after2\n";
  $y->foobar();
  echo "after3\n";
}
main();
