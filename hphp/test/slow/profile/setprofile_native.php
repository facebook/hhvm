<?php

function fb_setprofile_callback3($event, $name, $info) {
  echo "fb_setprofile_callback3 event=", $event, " name=", $name, " info=", $info, "\n";
}
function fb_setprofile_callback2($event, $name) {
  echo "fb_setprofile_callback2 event=", $event, " name=", $name, "\n";
}
function fb_setprofile_callback1($event) {
  echo "fb_setprofile_callback1 event=", $event, "\n";
}

fb_setprofile("fb_setprofile_callback1");

$x = false;
echo("x="); var_dump($x);

$algos="none";
echo("starting call to hash_algos\n");
if (true) {
  $algos = hash_algos();
}
echo("algos="); var_dump($algos);

echo("DONE!\n");
