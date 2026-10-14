<?hh

function expect_int(int $_)[]: void {}

function call_polymorphic((function()[_]: void) $f)[ctx $f]: void {
  $f();
}

final class Subject {
  public ?int $value = 1;

  public function dynamicFunctionValue(dynamic $f): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      // Currently accepted even though dynamic may hide a writing function.
      expect_int($this->value);
    }
  }

  public function likePureFunctionValue(~(function()[]: void) $f): void {
    if ($this->value !== null) {
      call_polymorphic($f);
      // Currently accepted even though the dynamic side may hide a writer.
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

  public function dynamicReceiver(dynamic $runner): void {
    if ($this->value !== null) {
      call_dependent($runner);
      // Currently accepted even though the receiver may require writing.
      expect_int($this->value);
    }
  }
}
