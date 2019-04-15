<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum E : string {
  A = "A";
  B = "B";
}

function testswitch(?E $eo):int {
  $eo = $eo ?? null;
  if ($eo === null)
    return 0;
  switch ($eo) {
    case E::A : return 1;
    case E::B : return 2;
  }
}
