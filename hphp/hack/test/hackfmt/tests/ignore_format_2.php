<?hh

function test(): vec<int> {
  $x =
    functionCall() +
    // hackfmt-ignore - this should be left as-is
    otherFunctionCall(vec[
      111111111111, 222222222222, 333333333333,
      444444444444, 555555555555, 666666666666,
      777777777777, 888888888888, 999999999999,
    ]);

  $y =
    functionCall() +
    // this should be formatted
    otherFunctionCall(vec[
      444444444444, 555555555555, 666666666666,
      777777777777, 888888888888, 999999999999,
      111111111111, 222222222222, 333333333333,
    ]);

  // hackfmt-ignore - this should be left as-is
  $z = otherFunctionCall(vec[111111111111, 222222222222, 333333333333, 444444444444, 555555555555, 666666666666, 777777777777, 888888888888, 999999999999,]);

  // This should be formatted
  $zzz = otherFunctionCall(vec[111111111111, 222222222222, 333333333333, 444444444444, 555555555555, 666666666666, 777777777777, 888888888888, 999999999999,]);

  f(() ==> {
    // hackfmt-ignore - this should be left as-is
    vec[
      $zzz, 200, 300,
      400, $zzz, 600,
      700, 800, $zzz,
    ];
  });

  // hackfmt-ignore - this should be left as-is
  return vec[
    $x, $y, $z,
    $z, $x, $y,
    $y, $z, $x,
  ];
}

function functionCall(): int {
  // hackfmt-ignore - this should be left as is
  $a = 1 +
    2 +
    3;

  // This should be formatted
  $b = 4 +
    5 +
    6;

  // hackfmt-ignore - this should left as-is
  return
    $a +
    $b;
}

function otherFunctionCall(vec<int> $v): int {
  $a = 0;

  // hackfmt-ignore - this should be left as-is
  foreach ($v as $key => $value)
  {
    $a += $value;
  }

  // This should be formatted
  foreach ($v as $key => $value)
  {
    $a += $key;
  }

  return $a;
}
