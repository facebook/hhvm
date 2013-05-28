<?php

function test($x) {
  $s_path = serialize($x);
  $filter = function ($rel) use ($s_path) {
    return $s_path;
  }
;
  var_dump($filter(0));
}
test('hello');
test(array(1,2,'foo'=>'bar'));
