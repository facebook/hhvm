<?php

final class Constants {
  public function gen() {
    yield 'foo';
  }
}

function main() {
  $g = Constants::gen();
  var_dump($g->current());
}

main();
