<?hh
class Parents {
   private $parents;
   public function __sleep() {
       return varray["parents"];
   }
}

class Child extends Parents {
    private $child;
    public function __sleep() {
        return array_merge(varray["child"], parent::__sleep());
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Child();
serialize($obj);
}
