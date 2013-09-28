<?php
class X {
  function test(self $s) {
    var_dump($s);
  }
}
X::test("hello");
