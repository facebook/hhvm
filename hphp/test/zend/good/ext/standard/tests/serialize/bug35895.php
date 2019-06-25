<?hh
class Parents {
   private $parents;
   public function __sleep() {
       return array("parents");
   }
}

class Child extends Parents {
    private $child;
    public function __sleep() {
        return array_merge(array("child"), parent::__sleep());
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Child();
serialize($obj);
}
