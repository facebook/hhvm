<?hh

class MySoap extends SoapClient {
   public $pub = 1;
   public function __dorequest($request, $location, $action, $version, $one_way=0) :mixed{
     $rp = parent::__dorequest($request, $location, $action, $version, $one_way);
     return $rp;
   }
 }

 function test($options) :mixed{
   return new MySoap(__DIR__.'/1809.wsdl', $options);
 }


 <<__EntryPoint>>
function main_1809() :mixed{
var_dump(test(dict['foo' => 'bar'])->pub);
}
