<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

VERIFY(is_bool(true));
VERIFY(!is_bool("test"));

VERIFY(is_int(123));
VERIFY(!is_int("123"));

VERIFY(is_integer(123));
VERIFY(!is_integer("123"));

VERIFY(is_long(123));
VERIFY(!is_long("123"));

VERIFY(is_double(123.4));
VERIFY(!is_double("123.4"));

VERIFY(is_float(123.4));
VERIFY(!is_float("123.4"));

VERIFY(is_numeric(123));
VERIFY(is_numeric("123.4"));
VERIFY(!is_numeric("e123.4"));

VERIFY(is_real(123.4));
VERIFY(!is_real("123.4"));

VERIFY(!is_string(123));
VERIFY(is_string("test"));

VERIFY(is_scalar(123));
VERIFY(is_scalar("test"));
VERIFY(!is_scalar(null));
VERIFY(!is_scalar(array(123)));

VERIFY(!is_array(123));
VERIFY(!is_array("test"));
VERIFY(is_array(array(123)));

VERIFY(!is_object(123));
VERIFY(!is_object("test"));
VERIFY(is_object(new stdclass));

VERIFY(!is_null(0));
VERIFY(!is_null(""));

VS(gettype(null), "NULL");
VS(gettype(0), "integer");
VS(gettype("test"), "string");
VS(gettype("hi"), "string");
VS(gettype(array()), "array");
VS(gettype(new stdclass), "object");

VS(intval("12"), 12);
VS(intval("12", 8), 10);

VS(doubleval("12.3"), 12.3);

VS(floatval("12.3"), 12.3);

VS(strval(123), "123");

{
  $v = "5bar";
  VERIFY(settype($v, "integer"));
  VS($v, 5);
}
{
  $v = true;
  VERIFY(settype($v, "string"));
  VS($v, "1");
}

$obj = new stdclass;
$obj->name = "value";
VS(serialize($obj), "O:8:\"stdClass\":1:{s:4:\"name\";s:5:\"value\";}");

$v = array("a"=>"apple","b"=>2,"c"=>array(1,"y",3));
VS(serialize($v),
   "a:3:{s:1:\"a\";s:5:\"apple\";s:1:\"b\";i:2;s:1:\"c\";a:3:{i:0;i:1;i:1;s:1:\"y\";i:2;i:3;}}");

{
  $v = unserialize("O:8:\"stdClass\":1:{s:4:\"name\";s:5:\"value\";}");
  VERIFY(is_object($v));
  VS($v->name, "value");
}
{
  $v = unserialize("O:8:\"stdClass\":1:{s:7:\"\0*\0name\";s:5:\"value\";}");
  VERIFY(is_object($v));
  VS($v->name, null);
}
{
  $v1 = array("a" => "apple", "b" => 2,"c" => array(1,"y",3));
  $v2 = unserialize("a:3:{s:1:\"a\";s:5:\"apple\";s:1:\"b\";i:2;s:1:\"c\";a:3:{i:0;i:1;i:1;s:1:\"y\";i:2;i:3;}}");
  VS($v1, $v2);
}

get_defined_vars();
