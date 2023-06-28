<?hh


class bar {
    public $z;

    public function __construct() {
        $this->z = new stdClass;
    }
    public function getZ() :mixed{
        return $this->z;
    }
}
<<__EntryPoint>> function main(): void {
var_dump(clone (new bar)->z);
var_dump(clone (new bar)->getZ());
}
