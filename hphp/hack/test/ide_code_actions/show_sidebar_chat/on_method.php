<?hh

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
    //                             ^ at-caret
  }
}

interface ISender<-TA, -TB> {}
