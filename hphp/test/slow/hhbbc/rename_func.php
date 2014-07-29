<?php

function foo() { return '123'; }
function bar() { return 'hehehe'; }

function main() {
  var_dump(foo());
  var_dump(bar());
  fb_rename_function('bar', 'baz');
  fb_rename_function('foo', 'bar');
  var_dump(bar());
}

main();
