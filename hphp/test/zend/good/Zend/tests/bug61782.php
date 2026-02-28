<?hh
abstract class BaseClass {
    abstract protected function __clone():mixed;
}

class MommasBoy extends BaseClass {
    protected function __clone() :mixed{
        echo __METHOD__, "\n";
    }
}

class LatchkeyKid extends BaseClass {
    public function __construct() {
        echo 'In ', __CLASS__, ":\n";
        $kid = new MommasBoy();
        $kid = clone $kid;
    }
    public function __clone() :mixed{}
}

<<__EntryPoint>> function main(): void {
$obj = new LatchkeyKid();
echo "DONE\n";
}
