<?hh

function autoloader($type, $name, $err) {
}

function foo() {
  echo "foo\n";
}

<<__EntryPoint>>
function main() {
  if (HH\execution_context() === "xbox") {
    return;
  }
  echo "main\n";

  fb_call_user_func_async(
    __FILE__,
    'foo'
  );
}
