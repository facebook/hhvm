<?hh

abstract class Machine {
  public function openDoors(): void {}
  public function closeDoors(): void {}
}

interface Fliers {
  public function fly(): bool;
}

trait Plane {
  require extends Machine;
  require implements Fliers;

  public function takeOff(): bool {
    $this->openDoors();
    $this->closeDoors();
    return $this->fly();
  }
}

interface HasEngine {
  public function startEngine(): void;
}

trait Startable {
  require extends Machine;
  require implements HasEngine;

  public function start(): void {
    $this->closeDoors();
    $this->startEngine();
  }
}

class AirBus extends Machine implements Fliers, HasEngine {
  use Plane;
  use Startable;

  public function fly(): bool {
    return true;
  }

  public function startEngine(): void {}
}
