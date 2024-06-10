<?hh

interface ISender<-TA, -TB> {}

final class SenderWA implements ISender<int, null> {
  public function __construct(protected int $a, protected null $b) {}
}



abstract class TestServiceBase<TA, TB> {
  public function __construct(
    protected TA $a,
    protected TB $b,
    ISender<TA, ?TB> $sender,
  ) {}
}

final class TestServiceWA extends TestServiceBase<int, string> {
  public function __construct(int $a, string $b, ISender<int, null> $sender) {
    parent::__construct($a, $b, $sender);
  }
}

final class TestFactory {

  public static function getService(int $a, string $b): TestServiceWA {
    return new TestServiceWA($a, $b, new SenderWA(1234567890, null));
  }
}
