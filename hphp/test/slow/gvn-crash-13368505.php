<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

$gmt = new DateTimeZone('GMT');

class XDate extends DateTime
{
  public function myFormat()
  {
    global $gmt;
    $return = parent::format("Y-m-d H:i:s");
    parent::setTimezone($gmt);
  }
}

function main() {
  global $gmt;

  $date = new XDate('now', $gmt);
  $date->myFormat();

  $date = new XDate('now', $gmt);
  $date->myFormat();
}

main();
echo "DONE\n";
