<?php

apc_add("ts", "TestString");
apc_add("ta", array("a" => 1, "b" => 2));
apc_add("ts", "NewValue");
apc_add("ta", array("newelement"));
apc_add(array("a" => 1, "b" => 2, "f" => "facebook"));
apc_add(array(
              "a" => array("b" => 1,
              "c" => array("d", "e")),
              "f" => array(1,2,3),
              "h" => "hello",
));

if (apc_fetch("ts") !== "TestString") {
  echo "no1\n";
}
if (apc_fetch("ta") !== array("a" => 1, "b" => 2)) {
  echo "no2\n";
}

if (apc_fetch("ts") !== "TestString") {
  echo "no3\n";
}
if (apc_fetch("ta") !== array("a" => 1, "b" => 2)) {
  echo "no4\n";
}
if (apc_fetch("a") !== 1) {
  echo "no19\n";
}
if (apc_fetch("f") !== "facebook") {
  echo "no20\n";
}

apc_add("texp", "TestString", 1);
sleep(1);
if (apc_fetch("texp") !== false) {
  echo "no5\n";
}

$ret = apc_store("foo", false);
if ($ret !== true) {
  echo "no6\n";
}
$ret = apc_add("foo", false);
if ($ret !== false) {
  echo "no7\n";
}
$ret = apc_fetch("foo", &$success);
if ($ret !== false) {
  echo "no8\n";
}
if ($success !== true) {
  echo "no9\n";
}
$ret = apc_fetch("bar", &$success);
if ($ret !== false) {
  echo "no10\n";
}
if ($success !== false) {
  echo "no11\n";
}

$map1 = array("foo" => false);
$ret = apc_fetch(array("foo"), &$success);
if ($ret !== $map1) {
  echo "no12\n";
}
$ret = apc_fetch(array("bar"), &$success);
if ($ret !== array()) {
  echo "no13\n";
}
if ($success !== false) echo "no14\n";
$ret = apc_fetch(array("foo", "bar"), &$success);
if ($ret !== $map1) echo "no15\n";
if ($success !== true) echo "no16\n";
$ret = apc_fetch(array("foo", "bar", "foo", "bar"), &$success);
if ($ret !== $map1) echo "no17\n";
if ($success !== true) echo "no18\n";

echo "ok\n";
