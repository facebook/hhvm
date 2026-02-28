<?hh

class MemoSensitiveData implements HH\IPureMemoizeParam {
  public function getPayload()[]: int {
    return 5;
  }
  public function getInstanceKey()[]: string {
    return 'strkey';
  }
}

final class IntContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = MemoSensitiveData;
  const ctx CRun = [zoned];
  public static async function setAsync(
    MemoSensitiveData $context,
    (function ()[_]: int) $f,
  )[zoned, ctx $f]: Awaitable<int> {
    echo 'Setting context to ' . $context->getPayload() . "\n";
    return await parent::runWithAsync($context, $f);
  }
  public static function getContext()[zoned]: MemoSensitiveData {
    return parent::get() as nonnull;
  }
}

async function addFive(): Awaitable<int> {
  await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
  return IntContext::getContext()->getPayload() + 5;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $result = await IntContext::setAsync(
    new MemoSensitiveData(),
    addFive<>,
  );
  var_dump($result);
}
