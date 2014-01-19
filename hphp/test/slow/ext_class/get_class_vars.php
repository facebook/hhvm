<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function __autoload($name) {
  echo "autoload $name\n";
}

var_dump(get_class_vars('nope'));
