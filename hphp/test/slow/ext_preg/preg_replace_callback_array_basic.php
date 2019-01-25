<?php
class Rep {
  public function __invoke() {
    return "d";
  }
}
class Foo {
  public static function rep($rep) {
    return "ok";
  }
}
function b() {
  return "b";
}

<<__EntryPoint>>
function main_preg_replace_callback_array_basic() {
var_dump(preg_replace_callback_array(
  array(
    "/a/" => 'b',
    "/b/" => function () { return "c"; },
    "/c/" => new Rep,
    '/d/' => array("Foo", "rep")), 'a'));
var_dump(preg_replace_callback_array(
  array(
    "/a/" => 'b',
    "/c/" => new Rep,
    "/b/" => function () { return "ok"; },
    '/d/' => array("Foo", "rep")), 'a'));
var_dump(preg_replace_callback_array(
  array(
    '/d/' => array("Foo", "rep"),
    "/c/" => new Rep,
    "/a/" => 'b',
    "/b/" => $_ ==> 'ok',
  ), 'a', -1, &$count));
var_dump($count);
}
