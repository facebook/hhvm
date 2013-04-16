<?php
$doc = dirname(__FILE__) . '/fbcufa.inc';
require_once $doc;
function main() {
  global $doc;
  echo "--------------------\n";
  $hnd = fb_call_user_func_async($doc, 'foo', array(1,2,3), 'blarg');
  $rv = fb_end_user_func_async($hnd);
  var_dump($rv);
  echo "--------------------\n";
  $hnd = fb_call_user_func_async($doc, 'C::bar', 42);
  $rv = fb_end_user_func_async($hnd);
  var_dump($rv);
  echo "--------------------\n";
  $hnd = fb_call_user_func_async($doc, array('C', 'bar'), 42);
  $rv = fb_end_user_func_async($hnd);
  var_dump($rv);
  echo "--------------------\n";
  $obj = new C;
  $obj->blah = 789;
  $hnd = fb_call_user_func_async($doc, array($obj, 'baz'), 73);
  $rv = fb_end_user_func_async($hnd);
  var_dump($rv);
  echo "--------------------\n";
  $hnd = fb_call_user_func_async($doc, 'doThrow');
  $rv = null;
  $caught = false;
  try {
    $rv = fb_end_user_func_async($hnd);
  } catch (Exception $e) {
    $caught = true;
  }
  if (!$caught) {
    echo "Exception was not thrown as expected\n";
  }
}
main();
