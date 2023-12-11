<?hh


<<__EntryPoint>>
function main_serialize() :mixed{
echo("\n*** string serialize-and-unserialize ***\n");
$ret = null;
var_dump(fb_unserialize(fb_serialize("test"), inout $ret));
var_dump($ret);
$ret = null;
var_dump(fb_unserialize(
  fb_serialize("test", FB_SERIALIZE_HACK_ARRAYS),
  inout $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n*** array serialize-and-unserialize ***\n");
$ret = null;
$unserialized = fb_unserialize(fb_serialize(vec["test"]), inout $ret);
var_dump($unserialized);
var_dump(is_darray($unserialized));
var_dump($ret);
$ret = null;

echo("\n* array with flag on serialize and unserialize *\n");
var_dump(fb_unserialize(  fb_serialize(vec["test"], FB_SERIALIZE_HACK_ARRAYS), inout $ret,   FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n* array with flag on only unserialize *\n");
$ret = null;
var_dump(fb_unserialize(
  fb_serialize(vec["test"]),
  inout $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n* array with flag on only serialize *\n");
$ret = null;
var_dump(fb_unserialize(
  fb_serialize(vec["test"], FB_SERIALIZE_HACK_ARRAYS),
  inout $ret,
));
var_dump($ret);


$ret = null;
var_dump(fb_unserialize(fb_serialize(vec["test", 42]), inout $ret));
var_dump($ret);

$ret = null;
var_dump(
  fb_unserialize(fb_serialize(dict["test" => 'testval', 42 => '42val']), inout $ret)
);
var_dump($ret);

echo("\n*** bad_unserialize ***\n");
$ret = null;
fb_unserialize("\t\374\003\310\001\270", inout $ret);
var_dump($ret);

echo("\n*** vec serialize-and-unserialize ***\n");
try {
  $ret = null;
  var_dump(fb_unserialize(fb_serialize(vec['foo', 42]), inout $ret));
  var_dump($ret);
} catch (Exception $e) {
  var_dump($e->getMessage());
}

$ret = null;
var_dump(
  fb_unserialize(fb_serialize(
    vec['foo', 42],
    FB_SERIALIZE_HACK_ARRAYS
  ),
  inout $ret,
  FB_SERIALIZE_HACK_ARRAYS
));
var_dump($ret);

echo("\n*** dict serialize-and-unserialize ***\n");
try {
  $ret = null;
  var_dump(fb_unserialize(fb_serialize(dict['foo' => 'f', 42 => '42val']), inout $ret));
  var_dump($ret);
} catch (Exception $e) {
  var_dump($e->getMessage());
}

$ret = null;
var_dump(
  fb_unserialize(
    fb_serialize(dict['foo' => 'f', 42 => '42val'], FB_SERIALIZE_HACK_ARRAYS),
    inout $ret,
    FB_SERIALIZE_HACK_ARRAYS
  )
);
var_dump($ret);
}
