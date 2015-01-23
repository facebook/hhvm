<?hh
/* Tests identity and basic helper functions */

async function waitForme(): Awaitable<string> {
  return 'Thank you for waiting';
}

function makeWaitHandles(): array<string,Awaitable<mixed>> {
  return array(
    'later' => HH\Asio\later(),
    'sleep' => HH\Asio\usleep(1),
    'call'  => HH\Asio\call('HH\Asio\later'),
    'val'   => HH\Asio\val('Hello World'),
    'user'  => waitForMe(),
  );
}

$handles = makeWaitHandles();
foreach($handles as $h) {
  var_dump($h->getName());
  var_dump($h->join());
}
