<?hh

final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static async function setAsync(
    int $context,
    (function ()[_]: int) $f,
  )[zoned, ctx $f]: Awaitable<int> {
    echo 'Setting context to ' . $context . "\n";
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: int {
    return parent::get() as nonnull;
  }
}

async function addFive(): Awaitable<int> {
  await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $result = await IntContext::setAsync(
    5,
    addFive<>,
  );
  var_dump($result);
}
