<?php

function check($f) {
  var_dump($f);
  var_dump(is_callable($f));
}

function main() {
    check('');
    check('main');
    check('blarblah');
}
main();
