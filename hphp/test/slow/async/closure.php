<?hh

function block() :mixed{
  return RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_NO_PENDING_IO,
    1,
  );
}
async function ret() :Awaitable<mixed>{
  return 0;
}

async function inFunc() :Awaitable<mixed>{
  $x = async function($a) { return $a; };
  $y = async function($a) { await block(); return $a; };
  $xval = await $x(1);
  $yval = await $y(2);
  var_dump($xval);
  var_dump($yval);
}


class F {
  static async function inMeth() :Awaitable<mixed>{
    $x = async function($a) { return $a; };
    $y = async function($a) { await block(); return $a; };
    $xval = await $x(3);
    $yval = await $y(4);
    var_dump($xval);
    var_dump($yval);
  }
}

<<__EntryPoint>>
function main_closure() :mixed{
;

$global = async function () {
  $x = await ret();
  var_dump($x);
};

HH\Asio\join($global());
HH\Asio\join(inFunc());
HH\Asio\join(F::inMeth());
}
