<?php

// Copyright 2004-2013 Facebook. All Rights Reserved.

function main($a, $b) {
  var_dump(is_object($a));
  var_dump(is_object($b));
  var_dump(is_object($a) xor is_object($b));
}

main(new stdclass, new stdclass);
