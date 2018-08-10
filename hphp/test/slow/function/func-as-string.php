<?hh

function main($x) {
  if (is_string($x)) {
    return $x;
  }
  return false;
}

var_dump(main('a string'));
var_dump(main(fun('main')));
