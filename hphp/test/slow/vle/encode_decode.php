<?hh

function nonBinaryStr(string $str): string {
  $result = "Len ". strlen($str).": ";
  for ($i = 0; $i < strlen($str); $i++) {
    if ($i > 0) {
      $result .= ",";
    }
    $result .= ord($str[$i]);
  }
  return $result;
}

function hackEncode(
  vec<int> $in,
  bool $use_zigzag,
): string {
  $ret = '';
  $prev = 0;
  foreach ($in as $value) {
    $v = $value - $prev;
    $prev = $value;
    if ($use_zigzag) {
      $v = $v < 0 ? (((-$v) << 1) - 1) : ($v << 1); // zigzag for negatives.
    } else {
      invariant($v >= 0, 'input must be sorted');
    }
    // build a new value in reverse order. the last char always has MSB=0, every other one has MSB=1.
    $enc = '';
    do {
      $enc = chr(($enc === '' ? 0x0 : 0x80) | ($v & 0x7F)).$enc;
      $v >>= 7;
    } while ($v);
    $ret .= $enc;
  }
  return $ret;
}

function hackDecode(
  string $out,
  bool $use_zigzag,
): vec<int> {
  $ret = vec[];
  for ($prev = 0, $i = 0, $max = strlen($out); $i < $max; ) {
    $v = 0;

    do {
      $ch = ord($out[$i]);
      $i++;
      $v = ($v << 7) + ($ch & 0x7F);
    } while ($ch > 0x7F);

    if ($use_zigzag) {
      $v = ($v & 0x1)
        ? -(($v + 1) >> 1)
        : ($v >> 1); // handle negatives (zigzag)
    }

    $ret[] = $v + $prev;
    $prev += $v;
  }
  return $ret;
}

function abtest(vec<int> $input, bool $zigzag): void {
  $vec1 = HH\vle_encode($input, $zigzag);
  $vec2 = hackEncode($input, $zigzag);
  $str1 = nonBinaryStr($vec1);
  $str2 = nonBinaryStr($vec2);
  if ($str1 === $str2) {
    echo "Encode Equal $str1\n";

    $dec1 = json_encode(HH\vle_decode($vec1, $zigzag));
    $dec2 = json_encode(hackDecode($vec2, $zigzag));
    if ($dec1 === $dec2) {
      echo "Decode Equal $dec1\n";
    } else {
      echo "Decode Not Equal $dec1 != $dec2\n";
    }
  } else {
    echo "Encode Not Equal $str1 != $str2\n";
  }
}

<<__EntryPoint>>
function main(): void {
  abtest(vec[1, 3, 5, 7, 9, 10, 11, 12, 13, 14], true);
  abtest(vec[1, 3, 5, 7, 9, 10, 11, 12, 13, 14], false);
  abtest(vec[1, 3, 5, 7, 9, 11, 13, 14, 300, 256*256*256], true);
  abtest(vec[1, 3, 5, 7, 9, 11, 13, 14, 300, 256*256*256], false);

  // Exception
  try {
    HH\vle_encode(vec[1, 2, 3, 4, -5, 6, 7, 8, 9, 10], false);
  } catch (Exception $ex) {
    echo $ex->getMessage()."\n";
  }
}
