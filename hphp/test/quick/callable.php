<?php

function check($f) {
  var_dump($f);
  var_dump(is_callable($f));
}

function typehint(callable $a) {
  var_dump('worked');
}

function id() {}

function main() {
    check('');
    check('id');
    check('blarblah');

    $cl = function() { return 'closure'; };
    typehint($cl);
    typehint('id');
}
main();
