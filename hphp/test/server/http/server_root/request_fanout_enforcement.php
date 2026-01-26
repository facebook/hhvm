<?hh

<<__DynamicallyCallable>>
function recursive_xbox(int $x) {
  var_dump($x);

  if ($x === 0) {
    return 1;
  }

  $awaitable = fb_gen_user_func_array(
    __FILE__,
    'recursive_xbox',
    vec[$x-1],
  );
  return HH\Asio\join($awaitable);
}

<<__EntryPoint>>
function main_request_fanout():void {

  $ctx = HH\execution_context();
  echo "In $ctx\n";


  if (HH\execution_context() !== "xbox") {
    $count = (int)(\HH\global_get('_GET')['count']);
    recursive_xbox($count);
  }

  echo "$ctx done.\n";
}
