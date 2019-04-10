<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type T2 = shape(
  ?'header' => ?dict<int, vec<string>>,
  ?'lines' =>
  ?dict<int, dict<int, vec<string>>>,
  ...
);

type T = shape(
  'errors' => T2,
  'warnings' => T2,
  ...
);

function testshapearray(
    ?T $response,
  ): void {
    $errors = $response['errors'] ?? shape();
    $header_result = ($errors['header'] ?? dict[]);
  }
