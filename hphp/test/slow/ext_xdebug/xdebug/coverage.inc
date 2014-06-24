<?php
function test_parse_time($f)
{
 if (preg_match("/[0-9]{12}/", $f)) {
    $type = "YYYYMMDDHHii";
  }
  else if (preg_match("/[0-9]{8}\ [0-9]{4}/", $f)) {
    $type = "YYYYMMDD HHii";
  }
  else if (preg_match("/[0-9]{4}-[0-9]{2}-[0-9]{2}/", $f)) {
    $type = "YYYY-MM-DD";
  }
  else {
    $type = "*unknown*";
  }

  echo "This is a $type format.\n";
}

$formats = array('2004-08-17 21:20', '20040817 2120');
foreach ($formats as $format) {
  test_parse_time($format);
}
