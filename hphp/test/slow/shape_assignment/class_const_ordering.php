<?hh
<<file: __EnableUnstableFeatures('shape_destructure')>>

class C {
  const string s1 = 'a';
  const string s2 = 'b';
  const string s3 = 'c';
}

function mk<T>(T $x1, T $x2, T $x3): shape(C::s1 => T, C::s2 => T, C::s3 => T) {
  return shape(C::s1 => $x1, C::s2 => $x2, C::s3 => $x3);
}

function mk2<T>(T $x1, T $x2, T $x3): shape(C::s1 => T, C::s2 => T, C::s3 => T) {
  return shape(C::s3 => $x3, C::s2 => $x2, C::s1 => $x1);
}

<<__EntryPoint>>
function main(): void {
  $s1 = mk(1, 2, 3);
  $s2 = mk(4, 5, 6);
  $s3 = mk(7, 8, 9);
  $s4 = mk($s1, $s2, $s3);

  $s5 = mk(10, 20, 30);
  $s6 = mk(40, 50, 60);
  $s7 = mk(70, 80, 90);
  $s8 = mk($s5, $s6, $s7);

  $s9 = mk(100, 200, 300);
  $s10 = mk(400, 500, 600);
  $s11 = mk(700, 800, 900);
  $s12 = mk($s9, $s10, $s11);

  $s13 = mk($s4, $s8, $s12);

  $v = vec[];
  for ($i = 0; $i < 27; $i++) {
    $v[] = 0;
  }

  shape(
    C::s1 => shape(
      C::s1 => shape(C::s1 => $v[0], C::s2 => $v[1], C::s3 => $v[2]),
      C::s2 => shape(C::s1 => $v[3], C::s2 => $v[4], C::s3 => $v[5]),
      C::s3 => shape(C::s1 => $v[6], C::s2 => $v[7], C::s3 => $v[8]),
    ),
    C::s2 => shape(
      C::s1 => shape(C::s1 => $v[9], C::s2 => $v[10], C::s3 => $v[11]),
      C::s2 => shape(C::s1 => $v[12], C::s2 => $v[13], C::s3 => $v[14]),
      C::s3 => shape(C::s1 => $v[15], C::s2 => $v[16], C::s3 => $v[17]),
    ),
    C::s3 => shape(
      C::s1 => shape(C::s1 => $v[18], C::s2 => $v[19], C::s3 => $v[20]),
      C::s2 => shape(C::s1 => $v[21], C::s2 => $v[22], C::s3 => $v[23]),
      C::s3 => shape(C::s1 => $v[24], C::s2 => $v[25], C::s3 => $v[26]),
    ),
  ) = $s13;
  var_dump($v);

  $s1 = mk2(1, 2, 3);
  $s2 = mk2(4, 5, 6);
  $s3 = mk2(7, 8, 9);
  $s4 = mk2($s1, $s2, $s3);

  $s5 = mk2(10, 20, 30);
  $s6 = mk2(40, 50, 60);
  $s7 = mk2(70, 80, 90);
  $s8 = mk2($s5, $s6, $s7);

  $s9 = mk2(100, 200, 300);
  $s10 = mk2(400, 500, 600);
  $s11 = mk2(700, 800, 900);
  $s12 = mk2($s9, $s10, $s11);

  $s13 = mk2($s4, $s8, $s12);

  $v = vec[];
  for ($i = 0; $i < 27; $i++) {
    $v[] = 0;
  }

  shape(
    C::s1 => shape(
      C::s1 => shape(C::s1 => $v[0], C::s2 => $v[1], C::s3 => $v[2]),
      C::s2 => shape(C::s1 => $v[3], C::s2 => $v[4], C::s3 => $v[5]),
      C::s3 => shape(C::s1 => $v[6], C::s2 => $v[7], C::s3 => $v[8]),
    ),
    C::s2 => shape(
      C::s1 => shape(C::s1 => $v[9], C::s2 => $v[10], C::s3 => $v[11]),
      C::s2 => shape(C::s1 => $v[12], C::s2 => $v[13], C::s3 => $v[14]),
      C::s3 => shape(C::s1 => $v[15], C::s2 => $v[16], C::s3 => $v[17]),
    ),
    C::s3 => shape(
      C::s1 => shape(C::s1 => $v[18], C::s2 => $v[19], C::s3 => $v[20]),
      C::s2 => shape(C::s1 => $v[21], C::s2 => $v[22], C::s3 => $v[23]),
      C::s3 => shape(C::s1 => $v[24], C::s2 => $v[25], C::s3 => $v[26]),
    ),
  ) = $s13;
  var_dump($v);
}
