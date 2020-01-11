<?hh // partial

// Regular class that is called xhp should still work
class xhp {}

function param_and_return(xhp $x): xhp {
  return $x;
}
