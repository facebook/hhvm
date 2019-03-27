<?php
/* the default gc root size is 10,000 */
for ($i=0; $i<9998; $i++) {
    $array = array(&$array);
    unset($array);
}

$matches = array("foo" => "bar", &$matches); /* this bucket will trigger the segfault */
$dummy   = array("dummy", &$dummy);          /* used to trigger gc_collect_cycles */
$matches[2] = $dummy;

str_replace("foo", "bar", "foobar", &$matches);
echo "okey";
