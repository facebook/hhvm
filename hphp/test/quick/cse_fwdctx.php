<?hh

final class HijriBirthday {
  public function convertSolarToLunar($year, $month, $date) :mixed{
    return HijriConverter::gregorianToHijri($year, $month, $date);
  }
}


class HijriConverter {
  public static function intPart($float) :mixed{
    if ($float < -0.0000001) {
      return ceil($float - 0.0000001);
    }
    return floor($float + 0.0000001);
  }

  public static function gregorianToHijri($y, $m, $d) :mixed{
    if ($y > 1700) {
      if (($y > 1582) || (($y == 1582) && ($m > 10)) ||
          (($y == 1582) && ($m == 10) && ($d > 14))) {
        $jd = self::intPart(
          (1461 * ($y + 4800 + self::intPart(($m - 14) / 12))) / 4
        ) + self::intPart(
          (367 * ($m - 2 - 12 * (self::intPart(($m - 14) / 12)))) / 12
        ) - self::intPart((3 * (
          self::intPart(($y + 4900 + self::intPart(($m - 14) / 12)) / 100)
        )) / 4) + $d - 32075;
      } else {
        $jd = 367 * $y - self::intPart(
            (7 * ($y + 5001 + self::intPart(($m - 9) / 7))) / 4
          ) + self::intPart((275 * $m) / 9) + $d + 1729777;
      }

      $l = $jd - 1948440 + 10632;
      $n = self::intPart(($l - 1) / 10631);
      $l = $l - 10631 * $n + 354;
      $j = (self::intPart((10985 - $l) / 5316)) *
        (self::intPart(( 50 * $l) / 17719)) +
        (self::intPart($l / 5670)) * (self::intPart((43 * $l) / 15238));
      $l = $l - (self::intPart((30 - $j) / 15)) *
        (self::intPart((17719 * $j) / 50)) -
        (self::intPart($j / 16)) * (self::intPart((15238 * $j) / 43)) + 29;
      $m = self::intPart((24 * $l) / 709);
      $d = $l - self::intPart((709 * $m) / 24);
      $y = 30 * $n + $j - 30;

      if ($d < 10)
        $d = "0".$d;

      if ($m < 10)
        $m = "0".$m;

      return vec[$y, $m, $d];
    }
    return vec[];
  }
}

function main() :mixed{
  var_dump((new HijriBirthday)->convertSolarToLunar(1983, (float)31, 7));
  var_dump((new HijriBirthday)->convertSolarToLunar(1983, (float)31, 7));
  var_dump((new HijriBirthday)->convertSolarToLunar(1983, (float)31, 7));
}
<<__EntryPoint>> function main_entry(): void {
main();
echo "done\n";
}
