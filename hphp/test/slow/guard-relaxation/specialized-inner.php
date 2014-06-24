<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function callee(HH\Map &$c) {

}

function main() {
  $c = HH\Map {};
  callee($c);
  var_dump($c);
}

main();
