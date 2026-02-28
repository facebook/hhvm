<?hh

async function gen() :AsyncGenerator<mixed,mixed,void>{
  await RescheduleWaitHandle::create(0, 0);
  throw new Exception("lol");
  yield 4;
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  ResumableWaitHandle::setOnFailCallback(($a, $b) ==> {
    asio_get_running();
  });
  await gen()->next();
}
