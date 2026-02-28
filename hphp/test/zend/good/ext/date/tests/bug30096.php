<?hh

abstract final class ZendGoodExtDateTestsBug30096 {
  public static $ts;
  public static $tsold;
}

function gm_date_check($hour, $minute, $second, $month, $day, $year) :mixed{
    echo "gmmktime($hour,$minute,$second,$month,$day,$year): ";

    ZendGoodExtDateTestsBug30096::$tsold = ZendGoodExtDateTestsBug30096::$ts;
    ZendGoodExtDateTestsBug30096::$ts = gmmktime($hour, $minute, $second, $month, $day, $year);

    echo ZendGoodExtDateTestsBug30096::$ts, " | gmdate('r', ".ZendGoodExtDateTestsBug30096::$ts."):", gmdate('r', ZendGoodExtDateTestsBug30096::$ts);
    if (ZendGoodExtDateTestsBug30096::$tsold > 0) {
        echo " | Diff: " . (ZendGoodExtDateTestsBug30096::$ts - ZendGoodExtDateTestsBug30096::$tsold);
    }
    echo "\n";
}

<<__EntryPoint>> function main(): void {
echo "no dst --> dst\n";
ZendGoodExtDateTestsBug30096::$ts = -1;
gm_date_check(01,00,00,03,27,2005);
gm_date_check(02,00,00,03,27,2005);
gm_date_check(03,00,00,03,27,2005);
gm_date_check(04,00,00,03,27,2005);

echo "\ndst --> no dst\n";
ZendGoodExtDateTestsBug30096::$ts = -1;
gm_date_check(01,00,00,10,30,2005);
gm_date_check(02,00,00,10,30,2005);
gm_date_check(03,00,00,10,30,2005);
gm_date_check(04,00,00,10,30,2005);
}
