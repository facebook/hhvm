<?hh // strict

namespace NS_interface_requirements_2;

abstract class Machine {
  public function openDoors(): void {
    return;
  }
  public function closeDoors(): void {
    return;
  }
}

interface Fliers {
  require extends Machine;
  public function fly(): bool;
}

class AirBus extends Machine implements Fliers {
  public function takeOff(): bool {
    $this->openDoors();
    $this->closeDoors();
    return $this->fly();
  }

  public function fly(): bool {
    return true;
  }
}

// Having this will not only cause a typechecker error, but also cause a fatal
// error in HHVM since we did not meet the interface requirement (extending
// Machine).

/*class Paper implements Fliers {
  public function fly(): bool {
    return false;
  }
}
*/

function main(): void {
  $ab = new AirBus();
  var_dump($ab);
  var_dump($ab->takeOff());
  $p = new AirBus();
  var_dump($p);
  var_dump($p->takeOff());
}

/* HH_FIXME[1002] call to main in strict*/
main();
