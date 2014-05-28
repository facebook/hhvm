<?php

class Counted implements Countable {
  public function count() { return 5; }
}
class NotCounted {
  public function count() { return 4; }
}
class BadCounted implements Countable {
  public function count() { throw new Exception("y u do dis"); }
}

function res() { return count(STDIN); }
function nvtw() { return count($GLOBALS); }
function staticArr() { return count(array(3,2,1,0)); }
function packed() { return count(array(3,2,1,new stdClass)); }
function vec() { return count(HH\Vector{5,7,8,3}); }
function counted_obj() { return count(new Counted); }
function not_counted_obj() { return count(new NotCounted); }
function bad_counted_obj() { return count(new BadCounted); }

function append($arr) {
  var_dump(count($arr));

  $new_arr = $arr;
  $new_arr[] = 1;

  var_dump(count($new_arr));
}

append(array(3,2));
append(HH\Vector{3,2});

var_dump(res());
var_dump(vec());
var_dump(nvtw() > 0);
var_dump(packed());
var_dump(staticArr());
var_dump(counted_obj());
var_dump(not_counted_obj());

try {
  var_dump(bad_counted_obj());
} catch (Exception $e) {
  var_dump($e->getMessage());
}
