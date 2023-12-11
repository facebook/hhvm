<?hh
class Parents {
   private $parents;
   public function __sleep() :mixed{
       return vec["parents"];
   }
}

class Child extends Parents {
    private $child;
    public function __sleep() :mixed{
        return array_merge(vec["child"], parent::__sleep());
    }
}
<<__EntryPoint>> function main(): void {
$obj = new Child();
serialize($obj);
}
