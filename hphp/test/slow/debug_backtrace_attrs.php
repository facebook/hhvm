<?hh

<<FooAttr>>
function foo() {
  var_dump(HH\Lib\Dict\map(
    debug_backtrace(DEBUG_BACKTRACE_INCLUDE_ATTRIBUTES),
    $frame ==> dict[
      'function' => $frame['function'] ?? null,
      'user_attributes' => $frame['user_attributes'] ?? null
    ],
  ));
}

<<FooAttr>>
function bar() {
  foo();
}

<<FooAttr("Bar")>>
function baz() {
  bar();
}

<<BarAttr(Alpha::class), BazAttr(12, 13)>>
function biz() {
  baz();
}

<<ABC("DEF", 5), __Memoize(#MakeICInaccessible)>>
function buz() {
  biz();
}

function f() {
  buz();
}

function g() {
  (() ==> { f(); })();
}

<<HAttr1, __Memoize(#NotKeyedByICAndLeakIC__DO_NOT_USE)>>
function h() {
  (<<CAttr1, CAttr2>> () ==> g())();
}

function k() {
  (<<KAttr1>> () ==> h())();
}

class Alpha {
  <<AlphaAttr1, __Memoize(#KeyedByIC)>>
  function am1() {
    k();
  }
  function am2() {
    (<<AlphaAttr2>> () ==> $this->am1())();
  }
}

class Beta {
  <<BAttr1>>
  static function bm1() {
    (new Alpha)->am2();
  }
  static function bm2() {
    (<<BAttr2>> () ==> self::bm1())();
  }
}

<<XAttr>>
async function x() {
  Beta::bm2();
}

async function y() {
  await (<<YAttr>> async () ==> await x())();
}

async function z() {
  await y();
}

<<__EntryPoint>>
async function main() {
  await z();
}
