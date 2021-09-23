<?hh

<<__SupportDynamicType>> class E< <<__NoRequireDynamic>> T as dynamic> {}

<<__SupportDynamicType>> class C {
   public ~E<int> $x;
   public function __construct() { $this->x = new E<int>(); }
}
