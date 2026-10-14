<?hh

function expect_int(int $_)[]: void {}

abstract class AbstractSubject {
  abstract const ctx CRun super [defaults];

  public ?int $value = 1;

  abstract public function run()[this::CRun]: void;

  public function callAbstract(): void {
    if ($this->value !== null) {
      $this->run();
      // Currently accepted even though a subclass may write properties.
      expect_int($this->value);
    }
  }
}

final class PureSubject {
  const ctx CRun = [];

  public ?int $value = 1;

  public function run()[this::CRun]: void {}

  public function callPure(): void {
    if ($this->value !== null) {
      $this->run();
      expect_int($this->value);
    }
  }
}

abstract class Runner {
  abstract const ctx CRun super [defaults];

  abstract public function run()[this::CRun]: void;
}

function call_dependent(Runner $runner)[$runner::CRun]: void {
  $runner->run();
}

final class DependentSubject {
  public ?int $value = 1;

  public function callAbstract(Runner $runner): void {
    if ($this->value !== null) {
      call_dependent($runner);
      // Currently accepted even though the context constant is abstract.
      expect_int($this->value);
    }
  }
}
