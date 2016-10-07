<?hh

echo("\n*** string serialize-and-unserialize ***\n");
$ret = null;
var_dump(fb_unserialize(fb_serialize("test"), $ret));
var_dump($ret);
$ret = null;
var_dump(fb_unserialize(
  fb_serialize("test", FB_SERIALIZE_HACK_ARRAYS),
  $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n*** array serialize-and-unserialize ***\n");
$ret = null;
var_dump(fb_unserialize(fb_serialize(array("test")), $ret));
var_dump($ret);
$ret = null;

echo("\n* array with flag on serialize and unserialize *\n");
var_dump(fb_unserialize(
  fb_serialize(array("test"), FB_SERIALIZE_HACK_ARRAYS),
  $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n* array with flag on only unserialize *\n");
$ret = null;
var_dump(fb_unserialize(
  fb_serialize(array("test")),
  $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n* array with flag on only serialize *\n");
$ret = null;
var_dump(fb_unserialize(
  fb_serialize(array("test"), FB_SERIALIZE_HACK_ARRAYS),
  $ret,
));
var_dump($ret);


$ret = null;
var_dump(fb_unserialize(fb_serialize(array("test", 42)), $ret));
var_dump($ret);

$ret = null;
var_dump(
  fb_unserialize(fb_serialize(array("test" => 'testval', 42 => '42val')), $ret)
);
var_dump($ret);

echo("\n*** bad_unserialize ***\n");
$ret = null;
fb_unserialize("\t\374\003\310\001\270", $ret);
var_dump($ret);

echo("\n*** vec serialize-and-unserialize ***\n");
$ret = null;
var_dump(fb_unserialize(fb_serialize(vec['foo', 42]), $ret));
var_dump($ret);
$ret = null;
var_dump(
  fb_unserialize(fb_serialize(
    vec['foo', 42],
    FB_SERIALIZE_HACK_ARRAYS
  ),
  $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n*** dict serialize-and-unserialize ***\n");
$ret = null;
var_dump(fb_unserialize(fb_serialize(dict['foo' => 'f', 42 => '42val']), $ret));
var_dump($ret);
$ret = null;
var_dump(
  fb_unserialize(
    fb_serialize(dict['foo' => 'f', 42 => '42val'], FB_SERIALIZE_HACK_ARRAYS),
    $ret,
    FB_SERIALIZE_HACK_ARRAYS
  )
);
var_dump($ret);
