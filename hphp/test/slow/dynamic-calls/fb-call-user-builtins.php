<?hh

function autoloader($type, $name, $err) :mixed{
}

<<__DynamicallyCallable>>
function foo() :mixed{
  echo "foo\n";
}

<<__EntryPoint>>
function main() :mixed{
  if (HH\execution_context() === "xbox") {
    return;
  }
  echo "main\n";

  fb_call_user_func_async(
    __FILE__,
    'foo'
  );
}
