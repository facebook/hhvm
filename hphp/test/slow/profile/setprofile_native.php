<?hh

function fb_setprofile_callback3($event, $name, $info) {
  echo "fb_setprofile_callback3 event=", $event, " name=", $name, " info=", $info, "\n";
}
function fb_setprofile_callback2($event, $name) {
  echo "fb_setprofile_callback2 event=", $event, " name=", $name, "\n";
}
function fb_setprofile_callback1($event) {
  echo "fb_setprofile_callback1 event=", $event, "\n";
}


<<__EntryPoint>>
function main_setprofile_native() {
fb_setprofile("fb_setprofile_callback1");

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
  "fb_setprofile_callback2",
  SETPROFILE_FLAGS_DEFAULT,
  vec['hash_init', 123],  // Non string input should not crash hhvm
);
$hash = hash_init('sha1');
echo("hash="); var_dump($hash);
}
