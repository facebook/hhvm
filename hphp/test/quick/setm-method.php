<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function main($o) {
  $o = $o->circle = $o;
  $o->foo();
}
main(new stdclass);
