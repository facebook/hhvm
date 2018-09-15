<?hh // strict
class :xhp {
}

class :something {
}

function xhp1() : :xhp {
  // UNSAFE
}
function xhp2() : void {
  $x = <something />;
}
