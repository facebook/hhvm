<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

var_dump(strtotime("now") > 0);
VS(strtotime("10 September 2000"), 968569200);
VS(strtotime("+1 day", 968569200), 968655600);
VS(strtotime("+1 week", 968569200), 969174000);
VS(strtotime("+1 week 2 days 4 hours 2 seconds", 968569200), 969361202);
VS(strtotime("next Thursday", 968569200), 968914800);
VS(strtotime("last Monday", 968569200), 968050800);

$str = "Not Good";
$timestamp = strtotime($str);
VS($timestamp, false);
VS(strtotime(""), false);

