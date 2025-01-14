<?hh

function lock3($name) {
  echo "$name\n";
  var_dump(HH\SimpleLock\try_lock('held'));
  var_dump(HH\SimpleLock\try_lock('held'));
  var_dump(HH\SimpleLock\try_lock('held'));
}

function thread_main() {
  lock3('thread');
}

<<__EntryPoint>>
async function main() {
  if (HH\execution_context() === "xbox") return;

  lock3('main');

  $t = fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );
  fb_end_user_func_async($t);

  HH\SimpleLock\unlock('held');

  $t = fb_call_user_func_async(
    __FILE__,
    'thread_main'
  );
  fb_end_user_func_async($t);

  lock3('main');
}

