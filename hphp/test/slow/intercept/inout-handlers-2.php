<?hh

function handler1($name, $target, inout $args, $ctx, inout $done) {
  $done = true;
  $args[0] = 'handler1';
}

class W {
  static function handler2($name, $target, inout $args, $ctx, inout $done) {
    $done = true;
    $args[0] = 'handler2';
  }

  function handler3($name, $target, inout $args, $ctx, inout $done) {
    $done = true;
    $args[0] = 'handler3';
  }

  function make_closure() {
    return ($name, $target, inout $args, $ctx, inout $done) ==> {
      spl_object_hash($this);
      $done = true;
      $args[0] = 'handler4';
    };
  }

  static function make_static_closure() {
    return ($name, $target, inout $args, $ctx, inout $done) ==> {
      $done = true;
      $args[0] = 'handler5';
    };
  }
}

function foo(&$x) { echo "fail!\n"; }
function bar(&$x) { echo "fail!\n"; }
function fiz(&$x) { echo "fail!\n"; }
function buz(&$x) { echo "fail!\n"; }
function biz(&$x) { echo "fail!\n"; }
function far(&$x) { echo "fail!\n"; }

<<__EntryPoint>>
function main() {
  $handler6 = ($name, $target, inout $args, $ctx, inout $done) ==> {
    $done = true;
    $args[0] = 'handler6';
  };
  fb_intercept('foo', 'handler1');
  fb_intercept('bar', 'W::handler2');
  fb_intercept('fiz', [new W, 'handler3']);
  fb_intercept('buz', (new W)->make_closure());
  fb_intercept('biz', W::make_static_closure());
  fb_intercept('far', $handler6);

  $x = 'fail'; foo(&$x); echo "foo: $x\n";
  $x = 'fail'; bar(&$x); echo "bar: $x\n";
  $x = 'fail'; fiz(&$x); echo "fiz: $x\n";
  $x = 'fail'; buz(&$x); echo "buz: $x\n";
  $x = 'fail'; biz(&$x); echo "biz: $x\n";
  $x = 'fail'; far(&$x); echo "far: $x\n";
}
