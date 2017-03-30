<?php
// Copyright 2004-present Facebook. All Rights Reserved.

$f = create_function('$x', 'return vec[$x];');
var_dump($f(123));
