<?hh // strict

function f3()[rx]: int {
  $a =  ()[rx] ==> {
    if (Rx\IS_ENABLED) {
      return 1;
    } else {
      return 2;
    }
  };
  return $a();
}
