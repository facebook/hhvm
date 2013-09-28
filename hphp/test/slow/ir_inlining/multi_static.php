<?php

final class Constants {
  static public function genA() {
    yield 'foo';
  }
  static public function genB($g) {
    $g->next();
    yield $g->current();
  }
  static public function genC($g) {
    $g->next();
    yield $g->current();
  }
  static public function genD($g) {
    $g->next();
    yield $g->current();
  }
  static public function genE($g) {
    $g->next();
    yield $g->current();
  }
  static public function genF($g) {
    $g->next();
    yield $g->current();
  }
  static public function genG($g) {
    $g->next();
    yield $g->current();
  }
  static public function genH($g) {
    $g->next();
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
  $g->next();
  var_dump($g->current());
}

main();
