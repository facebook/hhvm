<?hh // strict

namespace NS_context_dependent_constants;

function ComputeResult(): void {
  echo "Inside ComputeResult\n";
  echo "__FUNCTION__: "; var_dump(__FUNCTION__);
  echo "__METHOD__: "; var_dump(__METHOD__);
//  echo "__CLASS__: "; var_dump(__CLASS__);
//  echo "__TRAIT__: "; var_dump(__TRAIT__);
  echo "__NAMESPACE__: "; var_dump(__NAMESPACE__);
}

class Date {
  private string $name1 = __FUNCTION__;
  private string $name2 = __METHOD__;	// takes on "__construct" rather than ""

  public function __construct() {
    echo "Inside " . __METHOD__ . "\n";
    echo "__CLASS__: "; var_dump(__CLASS__);
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);
//    echo "__TRAIT__: "; var_dump(__TRAIT__);
    echo "__NAMESPACE__: "; var_dump(__NAMESPACE__);

    echo "\$name1: "; var_dump($this->name1);
    echo "\$name2: "; var_dump($this->name2);

    // ...
  }

  public function __destruct() {
    echo "Inside " . __METHOD__ . "\n";
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);

    // ...
  }

  public function setDay(int $day): void {
    echo "Inside " . __METHOD__ . "\n";
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);
    echo "__LINE__: "; var_dump(__LINE__);

    $this->priv1();
    Date::spf1();
  }

// public vs. private doesn't matter

  private function priv1(): void {
    echo "Inside " . __METHOD__ . "\n";
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);
  }

  static public function spf1(): void {
    echo "Inside " . __METHOD__ . "\n";
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);
  }

}

class DatePlus extends Date {
  public function xx(): void {
    echo "__CLASS__: "; var_dump(__CLASS__);
    echo "Inside " . __METHOD__ . "\n";
    echo "__FUNCTION__: "; var_dump(__FUNCTION__);
  }
}

function main(): void {
  echo "__LINE__: "; var_dump(__LINE__);

  echo "__FILE__: "; var_dump(__FILE__);

  echo "__DIR__: "; var_dump(__DIR__);
  var_dump(dirname(__FILE__));

  echo "__LINE__: "; var_dump(__LINE__);

  echo "__NAMESPACE__: "; var_dump(__NAMESPACE__);

  echo "-----------------------------------------\n";

  echo "Outside all classes\n";
//  echo "__CLASS__: "; var_dump(__CLASS__);

  echo "-----------------------------------------\n";

  echo "Outside all classes\n";
//  echo "__TRAIT__: "; var_dump(__TRAIT__);

  echo "-----------------------------------------\n";

  ComputeResult();

  echo "-----------------------------------------\n";

  $date1 = new Date();
  $date1->setDay(22);

  echo "-----------------------------------------\n";

  $datePlus1 = new DatePlus();
  $datePlus1->xx();
}

/* HH_FIXME[1002] call to main in strict*/
main();
