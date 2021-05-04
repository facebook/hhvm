namespace HH\__Private\MiniTest;

// Not currently using the HSL as it's used to test the HSL :)

class ExpectObj<T> {
  public function __construct(private T $value) {}
  private static function assert(bool $value, ?string $message = null): void {
    $message ??= 'Test assertion failed';
    invariant($value, '%s', $message);
  }

  public function toEqual(T $other, ?\HH\FormatString<\PlainSprintf> $message = null, mixed ...$args): void {
    if ($message !== null) {
      $message = \vsprintf($message, $args);
    }
    self::assert($this->value === $other, $message);
  }

  public function toNotEqual(T $other, ?string $message = null): void {
    self::assert($this->value !== $other, $message);
  }

  public function toBeNull<TInner>(): void where T = ?TInner{
    self::assert($this->value === null);
  }

  public function toNotBeNull<TInner>(): TInner where T = ?TInner {
    self::assert($this->value !== null);
    return $this->value as nonnull;
  }

  public function toBeTrue(?string $message = null): void where T = bool {
    self::assert($this->value, $message);
  }

  public function toBeFalse(?string $message = null): void where T = bool {
    self::assert(!$this->value, $message);
  }

  public function toContain<Tv>(Tv $val): void where T as Container<Tv> {
    self::assert(\in_array($val, $this->value, /* strict = */ true));
  }

  public function toContainKey<Tk as arraykey>(Tk $key): void where T as KeyedContainer<Tk, mixed> {
    self::assert(\array_key_exists($key, $this->value));
  }

  public function toContainSubstring(string $val): void where T = string {
    self::assert(\strpos($this->value, $val) !== false);
  }

  public function toNotContainSubstring(string $val): void where T = string {
    self::assert(\strpos($this->value, $val) === false);
  }

  // FIXME this really shouldn't be used in the HSL...
  public function toBePHPEqual(T $other): void {
    self::assert($this->value == $other);
  }

  public function toEqualWithDelta(T $other, float $delta): void where T = float {
    $diff = \abs($this->value - $other);
    self::assert($diff <= $delta);
  }

  public function toAlmostEqual(T $other): void where T = float {
    $this->toEqualWithDelta($other, 1.19e-07 * 4); // match fbexpect and gtest
  }

  public function toBeGreaterThan(T $other): void where T as num {
    self::assert($this->value > $other);
  }

  public function toBeGreaterThanOrEqualTo(T $other): void where T as num {
    self::assert($this->value >= $other);
  }

  public function toBeLessThan(T $other): void where T as num {
    self::assert($this->value < $other);
  }

  public function toBeLessThanOrEqualTo(T $other): void where T as num {
    self::assert($this->value <= $other);
  }

  public function toThrow<TEx as \Throwable>(
    classname<TEx> $ex,
    ?string $ex_message = null,
    ?string $_failure_message = null,
  ): TEx where T = (function(): mixed) {
    $thrown = null;
    try {
      $f = $this->value;
      $ret = $f();
      if ($ret is Awaitable<_>) {
        \HH\Asio\join($ret);
      }
    } catch (\Throwable $t) {
      $thrown = $t;
    }
    self::assert($thrown !== null);
    self::assert(\is_a($thrown, $ex));
    if ($ex_message !== null) {
      self::assert(\strpos($thrown?->getMessage() ?? '', $ex_message) !== false);
    }
    /* HH_FIXME[4110] verified with is_a() */
    return $thrown;
  }

  public function toBeInstanceOf(
    classname<mixed> $class
  ): void {
    self::assert(\is_a($this->value, $class));
  }

  public function notToThrow<TRet>(
  ): void where T = (function(): TRet) {
    $f = $this->value;
    $ret = $f();
    if ($ret is Awaitable<_>) {
      \HH\Asio\join($ret);
    }
  }

  public function toHaveSameContentAs(T $other): void where T as Container<mixed> {
    self::assert(\count($this->value) === \count($other), 'counts differ');
    foreach ($this->value as $v) {
      $found = false;
      foreach ($other as $vv) {
        if ($v === $vv) {
          $found = true;
          break;
        }
      }
      self::assert($found, 'Missing value: '.\var_export($v, true));
    }
  }
}
