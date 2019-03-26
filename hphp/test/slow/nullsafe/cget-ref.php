<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class thing {
  public $a;
}

function run($o, &$ref) {
  $ref = 'hi';
  $local = $o?->a;
  $local = 5;
  var_dump($ref);
}

<<__EntryPoint>>
function main() {
  $o = new thing();
  run($o, &$o->a);
}
