namespace HH\__Private\MiniTest;

abstract class HackTest {
  protected static function fail(string $message): noreturn {
    invariant_violation('%s', $message);
  }

  final public async function runAsync(): Awaitable<void> {
    \printf("--- %s::%s\n", static::class, __METHOD__);
    $rc = new \ReflectionClass($this);
    foreach ($rc->getMethods() as $method) {
      if (!$method->isPublic()) {
        continue;
      }
      $name = $method->getName();
      if (\strlen($name) < 4 || \substr($name, 0, 4) !== 'test') {
        continue;
      }
      \printf("-----   %s::%s()\n", static::class, $name);
      $dp = $method->getAttributeClass(DataProvider::class);
      if (!$dp) {
        $ret = $method->invoke($this);
        if ($ret is Awaitable<_>) {
          await $ret;
        }
      } else {
        try {
          $dp = $rc->getMethod($dp->provider);
        } catch (\ReflectionException $_) {
          \printf("!!!!! DataProvider %s::%s() does not exist\n", static::class, $dp->provider);
          exit(1);
        }
        if (!$dp->isPublic() && $dp->isStatic()) {
          \printf("!!!!! DataProvider %s::%s() must be public static\n", static::class, $dp->name);
          exit(1);
        }
        \printf("-----     Invoking DataProvider %s::%s()\n", static::class, $dp->name);
        $values = $dp->invoke($this);
        foreach($values as $k => $args) {
          \printf("-----     Invoking with data set %s...\n", \var_export($k, true));
          $ret = $method->invokeArgs($this, $args);
          if ($ret is Awaitable<_>) {
            await $ret;
          }
        }
      }
    }
    \printf("OK: %s\n", static::class);
  }
}
