<?hh // strict

class X {
  function __construct() {
    if (false) {
      var_dump($this?->x); // parse error
    }
  }
}


<<__EntryPoint>>
function main_nullsafe_prop_7() {
echo "not reached";

new X;
}
