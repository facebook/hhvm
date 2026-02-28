<?hh

class C {}

function handler($name, $_obj, inout $args) :mixed{
  echo "----HANDLER----\n";
  var_dump($name, $args);
  echo "---------------\n";
  return shape('callback' => bar<>);
}

<<__NEVER_INLINE>>
function foo(readonly C $c): readonly C { echo "in foo\n"; return $c;}
function bar(readonly C $c): readonly C { echo "in bar\n"; return $c;}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('foo', handler<>);
  readonly foo(new C);
  try { foo(new C); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
