<?hh

async function foo() :Awaitable<mixed>{
  return await gena(vec[
                      async { return 1; },
                      async { return 2; },
                      async { return 3; }
                    ]);
}

async function fooReschedule() :Awaitable<mixed>{
  return await gena(vec[
                      async { return 1; },
                      async { return 2; },
                      RescheduleWaitHandle::create(0,0)
                    ]);
}

async function fooError() :Awaitable<mixed>{
  return await gena(vec[
                      async { return 1; },
                      async { return 2; },
                      async { throw new Exception("oops"); }
                    ]);
}

async function bogusGena() :Awaitable<mixed>{
  return await gena(
    vec[async { return 1; }],
    vec[async { return 2; }],
    vec[async { return 3; }]
  );
}


<<__EntryPoint>>
function main_await_gena() :mixed{
var_dump(HH\Asio\join(foo()));
var_dump(HH\Asio\join(fooReschedule()));
try {
  var_dump(HH\Asio\join(fooError()));
} catch (Exception $e) { print $e->getMessage()."\n"; }
var_dump(HH\Asio\join(bogusGena()));
}
