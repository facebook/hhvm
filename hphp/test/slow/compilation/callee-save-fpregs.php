<?hh

$g = vec[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];

class X {
  function __toString() {
    global $g;
    $a0 = (float)$g[0];
    $a1 = (float)$g[1];
    $a2 = (float)$g[2];
    $a3 = (float)$g[3];
    $a4 = (float)$g[4];
    $a5 = (float)$g[5];
    $a6 = (float)$g[6];
    $a7 = (float)$g[7];
    $a8 = (float)$g[8];
    $a9 = (float)$g[9];
    $a10 = (float)$g[10];
    $a11 = (float)$g[11];
    $a12 = (float)$g[12];

    $x = ($a0 + $a1) + ($a2 + $a3) + ($a4 + $a5) +
         ($a6 + $a7) + ($a8 + $a9) + ($a10 + $a11) + $a12;

    return (string)$x;
  }
};

function test($a, $x) {
  return $a == $x;
}

$x = vec[new X];
for ($i = 0; $i < 5; $i++) {
  var_dump(test(vec["78"], $x));
}
