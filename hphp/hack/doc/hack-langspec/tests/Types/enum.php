<?hh // strict

namespace NS_enum_in_types_dir;

enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}

class C {
  const ControlStatus DEFAULT_STATE = ControlStatus::Stopped;
  private ControlStatus $prop = ControlStatus::Stopping;

  public function setProp(ControlStatus $val): void {
    $this->prop = $val;
  }

  public function getProp(): ControlStatus {
    return $this->prop;
  }
}

function main(): void {
}

/* HH_FIXME[1002] call to main in strict*/
main();
