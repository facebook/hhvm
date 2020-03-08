<?hh // strict

namespace NS_Vehicles;

require_once 'Aircraft.php';

class PassengerJet extends Aircraft {
  public function __construct(...) {}

  public function getMaxSpeed(): int {
    // implement method
    return 550;
  }

  public function getMaxAltitude(): int {
    // implement method
    return 30000;
  }

  // ...
}
