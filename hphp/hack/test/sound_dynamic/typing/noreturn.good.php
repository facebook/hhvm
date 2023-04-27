<?hh

<<__SupportDynamicType>>
function my_invariant_violation(string $fmt_args)[]: noreturn { throw new Exception(); }

class C3 {
  public function g(): ~vec<string> { return vec[]; }
  public function f(bool $b): int {
    if ($b) {
      return 4;
    } else {
      my_invariant_violation($this->g()[0]);
    }
  }
}

function bar(vec<int> $vi):noreturn {
  foo($vi); // Now we have no continuation
}

function foo(vec<int> $vi):noreturn {
  throw new Exception("A");
}

<<__SupportDynamicType>>
function getVecInt(): ~vec<int> { return vec[]; }

<<__SupportDynamicType>>
function fbar(vec<int> $vi):noreturn {
  // Here we do have a potential continuation: dynamic return
  ffoo(getVecInt());
}

<<__SupportDynamicType>>
function ffoo(vec<int> $vi):noreturn {
  throw new Exception("A");
}

<<__SupportDynamicType>>
class C {
  public static function mbar(vec<int> $vi):noreturn {
    self::mfoo(getVecInt());
  }

  <<__SupportDynamicType>>
  public static function mfoo(vec<int> $vi):noreturn {
    throw new Exception("A");
  }
}
