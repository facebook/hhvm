<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

GvnCrash13368505Php::$gmt = new DateTimeZone('GMT');

class XDate extends DateTime
{
  public function myFormat()
  {

    $return = parent::format("Y-m-d H:i:s");
    parent::setTimezone(GvnCrash13368505Php::$gmt);
  }
}

function main() {


  $date = new XDate('now', GvnCrash13368505Php::$gmt);
  $date->myFormat();

  $date = new XDate('now', GvnCrash13368505Php::$gmt);
  $date->myFormat();
}

main();
echo "DONE\n";

abstract final class GvnCrash13368505Php {
  public static $gmt;
}
