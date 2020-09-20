<?hh

function child_main(...$args): void {
  var_dump($args);
  throw new Exception("thrown from child");
}

<<__EntryPoint>>
function main(): void { if (hh\execution_context() === "xbox") return;
  HH\Asio\join(async {
    await fb_gen_user_func_array(
      __FILE__,
      'child_main',
      varray['args', 'go', 'here']
    );
  });
}
