<?hh

class MySoap extends SoapClient {
   public $pub = 1;
   public function __doRequest($request, $location, $action, $version, $one_way=0) {
     $rp = parent::__doRequest($request, $location, $action, $version, $one_way);
     return $rp;
   }
 }

 function test($options) {
   return new MySoap(__DIR__.'/1809.wsdl', $options);
 }


 <<__EntryPoint>>
function main_1809() {
var_dump(test(darray['foo' => 'bar'])->pub);
}
