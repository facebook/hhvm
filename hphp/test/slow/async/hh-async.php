<?hh
/* Tests identity and basic helper functions */

async function waitForme(): Awaitable<string> {
  return 'Thank you for waiting';
}

function makeWaitHandles(): darray<string,Awaitable<mixed>> {
  return darray[
    'later' => HH\Asio\later(),
    'sleep' => HH\Asio\usleep(1),
    'user'  => waitForMe(),
  ];
}


<<__EntryPoint>>
function main_hh_async() {
$handles = makeWaitHandles();
foreach($handles as $h) {
  var_dump(\HH\Asio\name($h));
  var_dump(\HH\Asio\join($h));
}
}
