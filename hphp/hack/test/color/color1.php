<?hh

// untyped
function f(dynamic $a):void {
  echo $a;
}

// partially typed return value
async function h():Awaitable<dynamic> {
  return 1;
}

async function i():Awaitable<void> {
  $x = await h();
  echo $x;
}

function strict(int $x): int {
  return $x;
}

function use_strict():void {
  echo strict(1);
}
