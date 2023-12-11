<?hh

function handler1($name, $target, inout $args) :mixed{
  $args[0] = 'handler1';
  return shape('value' => null);
}

class W {
  static function handler2($name, $target, inout $args) :mixed{
    $args[0] = 'handler2';
    return shape('value' => null);
  }

  function handler3($name, $target, inout $args) :mixed{
    $args[0] = 'handler3';
    return shape('value' => null);
  }

  function make_closure() :mixed{
    return ($name, $target, inout $args) ==> {
      spl_object_hash($this);
      $args[0] = 'handler4';
      return shape('value' => null);
    };
  }

  static function make_static_closure() :mixed{
    return ($name, $target, inout $args) ==> {
      $args[0] = 'handler5';
      return shape('value' => null);
    };
  }
}

<<__NEVER_INLINE>> function foo(inout $x) :mixed{ echo "fail!\n"; }
<<__NEVER_INLINE>> function bar(inout $x) :mixed{ echo "fail!\n"; }
<<__NEVER_INLINE>> function fiz(inout $x) :mixed{ echo "fail!\n"; }
<<__NEVER_INLINE>> function buz(inout $x) :mixed{ echo "fail!\n"; }
<<__NEVER_INLINE>> function biz(inout $x) :mixed{ echo "fail!\n"; }
<<__NEVER_INLINE>> function far(inout $x) :mixed{ echo "fail!\n"; }

<<__EntryPoint>>
function main() :mixed{
  $handler6 = ($name, $target, inout $args) ==> {
    $args[0] = 'handler6';
    return shape('value' => null);
  };
  fb_intercept2('foo', 'handler1');
  fb_intercept2('bar', 'W::handler2');
  fb_intercept2('fiz', vec[new W, 'handler3']);
  fb_intercept2('buz', (new W)->make_closure());
  fb_intercept2('biz', W::make_static_closure());
  fb_intercept2('far', $handler6);

  $x = 'fail'; foo(inout $x); echo "foo: $x\n";
  $x = 'fail'; bar(inout $x); echo "bar: $x\n";
  $x = 'fail'; fiz(inout $x); echo "fiz: $x\n";
  $x = 'fail'; buz(inout $x); echo "buz: $x\n";
  $x = 'fail'; biz(inout $x); echo "biz: $x\n";
  $x = 'fail'; far(inout $x); echo "far: $x\n";
}
