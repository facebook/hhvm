<?hh // strict

namespace NS_Vehicles;

require_once 'Vehicle.php';

abstract class Aircraft extends Vehicle {
  public abstract function getMaxAltitude(): int;
  // ...
}
