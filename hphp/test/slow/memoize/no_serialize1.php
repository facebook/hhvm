<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  <<__Memoize>> public function blah($x) :mixed{
    echo "blah...\n";
    return $x;
  }
}

function test() :mixed{
  $s = 'O:3:"Foo":1:{s:32:"' . "\0" . 'Foo' . "\0" . '$shared$multi$memoize_cache";D:1:{i:123;i:456;}}';
  $f = unserialize($s);
  var_dump($f->blah(123));
}

<<__EntryPoint>>
function main_no_serialize1() :mixed{
test();
}
