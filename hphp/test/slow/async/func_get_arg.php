<?hh

function block() { return RescheduleWaitHandle::create(1,1); };

async function num() {
  var_dump(func_num_args());
  await block();
  var_dump(func_num_args());
}

async function arg() {
  for ($i = 0; $i < func_num_args(); ++$i) {
    var_dump(func_get_arg($i));
  }
  await block();
  for ($i = 0; $i < func_num_args(); ++$i) {
    var_dump(func_get_arg($i));
  }
}

num("a", "b", "c")->join();
arg("e", "f")->join();
