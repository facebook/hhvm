<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function id($x) { return $x; }

class thinger {
  private
    $things = varray[],
    $otherThings = varray[],
    $updatedIDs = varray[];

  function thinger() {
    $this->things[3] = 'three';
    $this->things[44] = 'forty four';
    $this->things[45] = 'forty five';
    $this->fn = 'id';
  }

  private function process($history, inout $updated) {
    $updated = rand(0, 1) == 2;
    $fn = $this->fn;
    return $fn(varray[$history]);
  }

  public function done($init) {
    if ($this->things) {
      foreach ($this->things as $id => $history) {
        $updated = $init;
        $this->otherThings[$id] = $this->process($history, inout $updated);
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

<<__EntryPoint>>
function main_property_phi() {
main();
}
