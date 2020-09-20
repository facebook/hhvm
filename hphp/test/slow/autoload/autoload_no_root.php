<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function autoload_no_root(): void {
    HH\autoload_set_paths(
    darray[
      'function' => darray[
        'foo' => 'autoload_no_root.inc',
      ],
    ],
    '',
  );

  foo();
}
