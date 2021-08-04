<?hh

function compare_function($var1, $var2) {
  if (HH\Lib\Legacy_FIXME\eq($var1, $var2)) {
    return 0;
  } else if (HH\Lib\Legacy_FIXME\lt($var1, $var2)) {
    return -1;
  } else {
    return 1;
  }
}

