--TEST--
XHP idx Expression 02
--FILE--
<?php
class foo implements ArrayAccess {
  public function offsetExists($offset) { return true; }
  public function offsetGet($offset) { return $offset; }
  public function offsetSet($offset, $data) {}
  public function offsetUnset($offset) {}
}
$foo = new foo;

@var_dump(__xhp_idx($foo, 0));
@var_dump(__xhp_idx($foo, NULL));
@var_dump(__xhp_idx($foo, true));
@var_dump(__xhp_idx($foo, false));
@var_dump(__xhp_idx($foo, 'key'));
--EXPECT--
int(0)
NULL
bool(true)
bool(false)
string(3) "key"
