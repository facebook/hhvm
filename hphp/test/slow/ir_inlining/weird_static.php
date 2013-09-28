<?php

final class Constants {
  public function gen() {
    yield 'foo';
  }
}

function main() {
  $g = Constants::gen();
  $g->next();
  var_dump($g->current());
}

main();
