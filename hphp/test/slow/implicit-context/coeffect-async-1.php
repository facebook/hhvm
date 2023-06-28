<?hh

final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static async function setAsync(int $context, (function (): int) $f)[zoned] :Awaitable<mixed>{
    echo 'Setting context to ' . $context . "\n";
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: int {
    return parent::get() as nonnull;
  }
}

async function addFive()[zoned_with] :Awaitable<mixed>{
  await HH\Coeffects\backdoor_async(
    async ()[defaults] ==>
      await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0));
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  $result = await IntContext::setAsync(5, addFive<>); // FAIL
  var_dump($result);
  $result = await HH\Coeffects\enter_zoned_with_async('IntContext', 5, addFive<>);
  var_dump($result);
}
