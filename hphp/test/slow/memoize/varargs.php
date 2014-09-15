<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>>
function varargFn($a, ...) { return func_get_args(); }
