<?php
# Check for at least one HNI based function in get_defined_functions()
# and at least one IDL based function.
#
# Note to future self: One day (soon) this test will fail
# and Simon shall weep, for there are no more IDLs to convert.

$foundidl = false;
$foundhni = false;

foreach(get_defined_functions()['internal'] as $f) {
  $ishni = false;
  $rf = new ReflectionFunction($f);
  if ($attrs = $rf->getAttributes()) {
    foreach ($attrs as $name => $attr) {
      if (!strcasecmp($name, '__Native')) {
        $ishni = true;
        break;
      }
    }
  }
  if (!$ishni) {
    $foundidl = true;
  } else {
    $foundhni = true;
  }

  if ($foundhni && $foundidl) break;
}
var_dump($foundhni);
var_dump($foundidl);
