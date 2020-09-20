<?hh

function handler1($name, $target, inout $args, $ctx, inout $done) {
  $done = false;
  $args[0] = 'handler1';
}

class W {
  static function handler2($name, $target, inout $args, $ctx, inout $done) {
    $done = false;
    $args[0] = 'handler2';
  }

  function handler3($name, $target, inout $args, $ctx, inout $done) {
    $done = false;
    $args[0] = 'handler3';
  }

  function make_closure() {
    return ($name, $target, inout $args, $ctx, inout $done) ==> {
      spl_object_hash($this);
      $done = false;
      $args[0] = 'handler4';
    };
  }

  static function make_static_closure() {
    return ($name, $target, inout $args, $ctx, inout $done) ==> {
      $done = false;
      $args[0] = 'handler5';
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
  $handler6 = ($name, $target, inout $args, $ctx, inout $done) ==> {
    $done = false;
    $args[0] = 'handler6';
  };
  fb_intercept('foo', 'handler1');
  fb_intercept('bar', 'W::handler2');
  fb_intercept('fiz', varray[new W, 'handler3']);
  fb_intercept('buz', (new W)->make_closure());
  fb_intercept('biz', W::make_static_closure());
  fb_intercept('far', $handler6);

  $x = 'fail'; foo(inout $x);
  $x = 'fail'; bar(inout $x);
  $x = 'fail'; fiz(inout $x);
  $x = 'fail'; buz(inout $x);
  $x = 'fail'; biz(inout $x);
  $x = 'fail'; far(inout $x);
}
