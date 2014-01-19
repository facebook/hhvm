<?php
// Copyright 2004-present Facebook. All Rights Reserved.
function get_region($s) {
  echo "get_region\n";
  return $s ? 1 : 0;
}
function get_local_cluster_id() {
  return c::thing();
}
function get_local_region() {
  return get_region(get_local_cluster_id());
}
class c {
  public function __construct() {
    $local1 = new stdclass;
    $local2 = array();
    get_local_region();
  }

  public static function thing() {
    return rand() ? 1 : 0;
  }
}

new c;
echo "done\n";
