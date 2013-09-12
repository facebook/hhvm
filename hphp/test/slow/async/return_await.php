<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function f1($a) { return "f1".$a; }
async function f2($a) { return await f1("f2".$a); }

async function f3($a) {
  await block();
  return await f2("f3, ".$a);
}

var_dump(f2(".")->join());
var_dump(f3("!")->join());
