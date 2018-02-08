<?hh

async function foo() {
  return await gena(array(
                      async { return 1; },
                      async { return 2; },
                      async { return 3; }
                    ));
}

async function fooReschedule() {
  return await gena(array(
                      async { return 1; },
                      async { return 2; },
                      RescheduleWaitHandle::create(0,0)
                    ));
}

async function fooError() {
  return await gena(array(
                      async { return 1; },
                      async { return 2; },
                      async { throw new Exception("oops"); }
                    ));
}

async function bogusGena() {
  return await gena(
    array(async { return 1; }),
    array(async { return 2; }),
    array(async { return 3; })
  );
}

var_dump(HH\Asio\join(foo()));
var_dump(HH\Asio\join(fooReschedule()));
try {
  var_dump(HH\Asio\join(fooError()));
} catch (Exception $e) { print $e->getMessage()."\n"; }
var_dump(HH\Asio\join(bogusGena()));
