<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  private $cache = array();
  public function get($key1, $key2, $key3) {
    return isset($this->cache[$key1][$key2][$key3]);
  }
}


<<__EntryPoint>>
function main_isset() {
$x = new A();
var_dump($x->get('abc', 'def', 'ghi'));
}
