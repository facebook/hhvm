<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo($a, $b) :mixed{
  try {
    throw new Exception('wtf');
  } catch (Exception $e) {
    throw new Exception('nope');
  }
}

async function bar($a, $b) :Awaitable<mixed>{
  try {
    throw new Exception('wtf');
  } catch (Exception $e) {
    throw new Exception('nope');
  }
}

function handler($name, $obj_or_cls, inout $args) :mixed{
  throw new Exception('yep');
}

<<__EntryPoint>>
function main(): void {
  $funcs = vec['foo', 'bar'];
  foreach ($funcs as $func) {
    fb_intercept2($func, 'handler');
    try {
      $func(1, 2);
      echo "lol\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
  }
}
