<?hh

trait Herp {
  function foo(inout $x, inout $y) {
    $r = $x + $y;
    $x = 'Herp';
    $y = 42;
    return "Herp::foo($r)";
  }

  function bar(&$a) {
    $a = 'Herp&';
    return 'Herp::bar';
  }

  function herp(inout $a) {
    $a = 'Herp Herp';
    return 'Herp::herp';
  }

  static function s1(inout $a) { return 'Herp::s1'; }
  static function s2(inout $a) { return 'Herp::s2'; }
}

trait Derp {
  function foo($x, inout $y) {
    $r = $x + $y;
    $x = 'Derp';
    $y = 24;
    return "Derp::foo($r)";
  }

  function bar(&$a) {
    $a = 'Derp&';
    return 'Derp::bar';
  }

  function derp(&$a) {
    $a = 'Derp Derp';
    return 'Derp::derp';
  }

  static function s1(inout $a) { return 'Derp::s1'; }
  static function s2($a) { return 'Derp::s2'; }
}

class HerpDerp {
  use Herp, Derp {
    Herp::foo insteadof Derp;
    Derp::bar insteadof Herp;
    Herp::herp as apple;
    Derp::derp as potato;
    Herp::s1 insteadof Derp;
    Herp::s2 insteadof Derp;
    Derp::s2 as orange;
  }
}

function main() {
  $o = new HerpDerp;
  list($x, $y) = array(60, 40);
  $r = $o->foo(inout $x, inout $y);
  var_dump($r, $x, $y);
  $r = $o->bar(inout $x);
  var_dump($r, $x);
  $r = $o->apple(inout $x);
  var_dump($r, $x);
  $r = $o->potato(inout $x);
  var_dump($r, $x);

  var_dump(HerpDerp::s1(inout $x), $x);
  var_dump(HerpDerp::s2($x), $x);
  var_dump(HerpDerp::s2(inout $x));
  var_dump(HerpDerp::orange($x));
}

main();
