<?php

class FluentObj {
  private
    $first,
    $second,
    $third,
    $fourth,
    $fifth;

  public function __construct($first) {
    $this->first = $first;
  }

  public function setSecond($x) {
    $this->second = $x;
    return $this;
  }
  public function setThird($x) {
    $this->third = $x;
    return $this;
  }
  public function setFourth($x) {
    $this->fourth = $x;
    return $this;
  }
  public function setFifth($x) {
    $this->fifth = $x;
    return $this;
  }

  public function getValue() {
    return $this->first;
  }
}

function main() {
  $k = (new FluentObj('a'))
    ->setSecond('b')
    ->setThird('c')
    ->setFourth('d')
    ->setFifth('e')
    ->getValue();
  echo $k;
  echo "\n";
}
main();
