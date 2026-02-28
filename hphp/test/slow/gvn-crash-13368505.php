<?hh

class XDate extends DateTime
{
  public function myFormat()
:mixed  {

    $return = parent::format("Y-m-d H:i:s");
    parent::setTimezone(GvnCrash13368505Php::$gmt);
  }
}

function main() :mixed{


  $date = new XDate('now', GvnCrash13368505Php::$gmt);
  $date->myFormat();

  $date = new XDate('now', GvnCrash13368505Php::$gmt);
  $date->myFormat();
}

abstract final class GvnCrash13368505Php {
  public static $gmt;
}
<<__EntryPoint>>
function entrypoint_gvncrash13368505(): void {
  // Copyright 2004-present Facebook. All Rights Reserved.

  GvnCrash13368505Php::$gmt = new DateTimeZone('GMT');

  main();
  echo "DONE\n";
}
