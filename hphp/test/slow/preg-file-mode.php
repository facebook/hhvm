<?php

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_preg_file_mode() {
var_dump(preg_replace('/(.*)/e', '(vec[1, 2, 3])[1]', ''));
}
