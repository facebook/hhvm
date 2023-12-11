<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function id($x) :mixed{ return $x; }

class thinger {
  private
    $things = vec[],
    $otherThings = vec[],
    $updatedIDs = vec[];

  function thinger() :mixed{
    $this->things[3] = 'three';
    $this->things[44] = 'forty four';
    $this->things[45] = 'forty five';
    $this->fn = id<>;
  }

  private function process($history, inout $updated) :mixed{
    $updated = rand(0, 1) == 2;
    $fn = $this->fn;
    return $fn(vec[$history]);
  }

  public function done($init) :mixed{
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

function main() :mixed{
  $t = new thinger;
  for ($i = 0; $i < 20; ++$i) {
    $t->done(false);
  }
}

<<__EntryPoint>>
function main_property_phi() :mixed{
main();
}
