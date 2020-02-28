<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

HH\autoload_set_paths(
  darray[
    'function' => darray[
      'foo' => 'autoload_no_root.inc',
    ],
  ],
  '',
);

<<__EntryPoint>>
function autoload_no_root(): void {
  foo();
}
