<?hh

function handler1($name, $target, inout $args) {
  $args[0] = 'handler1';
  return shape();
}

class W {
  static function handler2($name, $target, inout $args) {
    $args[0] = 'handler2';
    return shape();
  }

  function handler3($name, $target, inout $args) {
    $args[0] = 'handler3';
    return shape();
  }

  function make_closure() {
    return ($name, $target, inout $args) ==> {
      spl_object_hash($this);
      $args[0] = 'handler4';
      return shape();
    };
  }

  static function make_static_closure() {
    return ($name, $target, inout $args) ==> {
      $args[0] = 'handler5';
      return shape();
    };
  }
}

function foo(inout $x) { echo "foo: $x\n"; }
function bar(inout $x) { echo "bar: $x\n"; }
function fiz(inout $x) { echo "fiz: $x\n"; }
function buz(inout $x) { echo "buz: $x\n"; }
function biz(inout $x) { echo "biz: $x\n"; }
function far(inout $x) { echo "far: $x\n"; }

<<__EntryPoint>>
function main() {
  $handler6 = ($name, $target, inout $args) ==> {
    $args[0] = 'handler6';
    return shape();
  };
  fb_intercept2('foo', 'handler1');
  fb_intercept2('bar', 'W::handler2');
  fb_intercept2('fiz', varray[new W, 'handler3']);
  fb_intercept2('buz', (new W)->make_closure());
  fb_intercept2('biz', W::make_static_closure());
  fb_intercept2('far', $handler6);

  $x = 'fail'; foo(inout $x);
  $x = 'fail'; bar(inout $x);
  $x = 'fail'; fiz(inout $x);
  $x = 'fail'; buz(inout $x);
  $x = 'fail'; biz(inout $x);
  $x = 'fail'; far(inout $x);
}
