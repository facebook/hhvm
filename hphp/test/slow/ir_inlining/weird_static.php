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


<<__EntryPoint>>
function main_weird_static() {
main();
}
