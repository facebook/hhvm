<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class thing {
  private
    $location,
    $kittens,
    $paper,
    $puppies;

  public function __construct() {
    $this->location = '0110111001101';
    $this->paper = dict[
      'a' => true,
      'b' => false,
      'c' => true,
      'd' => false,
    ];
    $this->kittens = dict[
      'a' => 1,
      'b' => 1,
      'c' => 1,
      'd' => 1,
    ];
    $this->puppies = dict[
      'a' => vec[12, 34],
      'b' => vec[56, 78],
      'c' => vec[90, 12],
      'd' => vec[34, 56],
    ];
  }

  public function teleport() :mixed{
    $arr = dict[];
    $location = $this->location;

    $offset = 0;
    foreach ($this->paper as $field => $bool) {
      if ($bool) {
        $translation = $location[$offset++];
        $arr[$field] = $this->puppies[$field][(int)$translation];
      } else {
        $arr[$field] = substr($location, $offset,
                              $this->kittens[$field]);
        $offset += $this->kittens[$field];
      }
    }
    return $arr;
  }
}


<<__EntryPoint>>
function main_pgo_loadelim() :mixed{
$t = new thing;
for ($i = 0; $i < 10; ++$i) {
  var_dump($t->teleport());
}
}
