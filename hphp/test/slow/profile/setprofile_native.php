<?hh

class SetProfileSimpleObject {
  public int $mine = 4;

  <<__NEVER_INLINE>>
  public static function foo() :mixed{
    $i = 100;
    $j = 200;
  }

  <<__NEVER_INLINE>>
  public static function bar() :mixed{
    $i = 100;
    $j = 200;
  }

  <<__NEVER_INLINE>>
  public function test() :mixed{
    SetProfileSimpleObject::bar();
    SetProfileSimpleObject::foo();
  }

  <<__NEVER_INLINE>>
  public static function testStatic() :mixed{
    self::foo();
    self::bar();
  }
}

function fb_setprofile_callback3($event, $name, $info) :mixed{
  echo "fb_setprofile_callback3 event=", $event, " name=", $name, " info=", $info, "\n";
}
function fb_setprofile_callback2($event, $name) :mixed{
  echo "fb_setprofile_callback2 event=", $event, " name=", $name, "\n";
}
function fb_setprofile_callback1($event) :mixed{
  echo "fb_setprofile_callback1 event=", $event, "\n";
}
function fb_setprofile_this($event, $name, $info) :mixed{
  if ($event === "enter") {
    $is_null = idx($info, "this_obj") === null ? 0 : 1;
    echo "fb_setprofile_this this_obj=", $is_null, "\n";
    echo "fb_setprofile_this mine=", idx($info, "this_obj")?->mine, "\n";
  }
}
function fb_setprofile_file_line($event, $name, $info) :mixed{
  if ($event === "enter") {
    $file_parts = explode("/", idx($info, "file"));
    $file = $file_parts[count($file_parts) - 1];
    $line = idx($info, "line");
    echo "fb_setprofile_file_line name=", $name, " file=", $file, " line=", $line, "\n";
  }
}


<<__EntryPoint>>
function main_setprofile_native() :mixed{
fb_setprofile(fb_setprofile_callback1<>);

$x = false;
echo("x="); var_dump($x);

$algos="none";
echo("starting call to hash_init\n");
if (true) {
  $hash = hash_init('md5');
}
echo("hash="); var_dump($hash);

echo("DONE!\n");

fb_setprofile(null);
fb_setprofile(
  fb_setprofile_callback2<>,
  SETPROFILE_FLAGS_DEFAULT,
  vec['hash_init', 123],  // Non string input should not crash hhvm
);
$hash = hash_init('sha1');
echo("hash="); var_dump($hash);

fb_setprofile(null);
fb_setprofile(
  fb_setprofile_this<>,
  SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_FRAME_PTRS |
    SETPROFILE_FLAGS_THIS_OBJECT__MAY_BREAK,
  vec['SetProfileSimpleObject::test', 'SetProfileSimpleObject::testStatic']
);
$obj = new SetProfileSimpleObject();
$obj->test();
SetProfileSimpleObject::testStatic();
fb_setprofile(null);
fb_setprofile(
  fb_setprofile_file_line<>,
  SETPROFILE_FLAGS_DEFAULT | SETPROFILE_FLAGS_FILE_LINE,
  vec['SetProfileSimpleObject::test', 'SetProfileSimpleObject::testStatic', 'SetProfileSimpleObject::foo', 'SetProfileSimpleObject::bar']
);
$obj = new SetProfileSimpleObject();
$obj->test();
SetProfileSimpleObject::testStatic();
echo "test done\n";
}
