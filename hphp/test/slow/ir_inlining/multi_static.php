<?php

final class Constants {
  static public function genA() {
    yield 'foo';
  }
  static public function genB($g) {
    yield $g->current();
  }
  static public function genC($g) {
    yield $g->current();
  }
  static public function genD($g) {
    yield $g->current();
  }
  static public function genE($g) {
    yield $g->current();
  }
  static public function genF($g) {
    yield $g->current();
  }
  static public function genG($g) {
    yield $g->current();
  }
  static public function genH($g) {
    yield $g->current();
  }
}

function main() {
  $g = Constants::genA();
  $g = Constants::genB($g);
  $g = Constants::genC($g);
  $g = Constants::genD($g);
  $g = Constants::genE($g);
  $g = Constants::genF($g);
  $g = Constants::genG($g);
  $g = Constants::genH($g);
  var_dump($g->current());
}

main();
