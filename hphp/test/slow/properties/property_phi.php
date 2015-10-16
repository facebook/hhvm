<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function id($x) { return $x; }

class thinger {
  private
    $things = array(),
    $otherThings = array(),
    $updatedIDs = array();

  function thinger() {
    $this->things[3] = 'three';
    $this->things[44] = 'forty four';
    $this->things[45] = 'forty five';
    $this->fn = 'id';
  }

  private function process($history, &$updated) {
    $updated = rand(0, 1) == 2;
    $fn = $this->fn;
    return $fn([$history]);
  }

  public function done($init) {
    if ($this->things) {
      foreach ($this->things as $id => $history) {
        $updated = $init;
        $this->otherThings[$id] = $this->process($history, $updated);
        if ($updated) {
          $this->updatedIDs[$id] = 1;
        }
      }
    }
    echo "done\n";
  }
}

function main() {
  $t = new thinger;
  for ($i = 0; $i < 20; ++$i) {
    $t->done(false);
  }
}
main();
