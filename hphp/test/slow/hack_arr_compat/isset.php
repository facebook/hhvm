<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  private $cache = dict[];
  public function get($key1, $key2, $key3) :mixed{
    return isset($this->cache[$key1][$key2][$key3]);
  }
}


<<__EntryPoint>>
function main_isset() :mixed{
$x = new A();
var_dump($x->get('abc', 'def', 'ghi'));
}
