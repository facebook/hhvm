<?php

class TrueFilter extends php_user_filter {
  function onCreate() {
    print("onCreate\n");
    return true;
  }
}

class FalseFilter extends php_user_filter {
  function onCreate() {
    print("onCreate\n");
    return false;
  }
}

class NullFilter extends php_user_filter {
  function onCreate() {
    print("onCreate\n");
    return null;
  }
}

function main() {
  $filters = array(
    'TrueFilter',
    'FalseFilter',
    'NullFilter'
  );
  foreach ($filters as $filter) {
    printf("---%s---\n", $filter);
    stream_filter_register($filter, $filter);
    print("r\n");
    $fr = fopen('php://memory', 'r');
    stream_filter_append($fr, $filter);
    print("r+\n");
    $frp = fopen('php://memory', 'r+');
    stream_filter_append($frp, $filter);
    print("w\n");
    $fw = fopen('php://memory', 'w');
    stream_filter_append($fw, $filter);
  }
}

main();
