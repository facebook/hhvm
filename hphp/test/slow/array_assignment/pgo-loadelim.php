<?php
// Copyright 2004-present Facebook. All Rights Reserved.

class thing {
  private
    $location,
    $kittens,
    $paper,
    $puppies;

  public function __construct() {
    $this->location = '0110111001101';
    $this->paper = array(
      'a' => true,
      'b' => false,
      'c' => true,
      'd' => false,
    );
    $this->kittens = array(
      'a' => 1,
      'b' => 1,
      'c' => 1,
      'd' => 1,
    );
    $this->puppies = array(
      'a' => [12, 34],
      'b' => [56, 78],
      'c' => [90, 12],
      'd' => [34, 56],
    );
  }

  public function teleport() {
    $arr = array();
    $location = $this->location;

    $offset = 0;
    foreach ($this->paper as $field => $bool) {
      if ($bool) {
        $translation = $location[$offset++];
        $arr[$field] = $this->puppies[$field][$translation];
      } else {
        $arr[$field] = substr($location, $offset,
                              $this->kittens[$field]);
        $offset += $this->kittens[$field];
      }
    }
    return $arr;
  }
}

$t = new thing;
for ($i = 0; $i < 10; ++$i) {
  var_dump($t->teleport());
}
