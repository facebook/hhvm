<?hh

$str = "foo" . 100 . "bar";

$str = "foo" . 0755 . "bar";

$str = "foo" . 0xdeadbeef . "bar";

$str = "foo" . 0b01100010 . "bar";

$str = "foo" . 100.0 . "bar";

$style = "background-size:" . 100 * $pages . "% auto;"
  . "padding-bottom:" . $height / $width * 100 . "%;"
  . "background-image: url(" . $background . ")";
