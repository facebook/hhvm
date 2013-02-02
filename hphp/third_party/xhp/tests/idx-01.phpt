--TEST--
XHP idx Expression 01
--INI--
xhp.idx_expr=1
--FILE--
<?php
function foo() {
  return array(
    'bar' => 'etc',
  );
}
echo foo()['bar'];
--EXPECT--
etc
