<?hh

class SOAP_productDetailsType {
    public $id = 0;
}

class SOAP_GetProductsRequest {
    public $time = '';
}

class SOAP_GetProductsResponse {
    public $products;
    function __construct(){
        $this->products = new SOAP_productDetailsType();
        
    }
}

class SOAP_Admin {
    public function GetProducts($time):mixed{
        return new SOAP_GetProductsResponse();
    }
}
<<__EntryPoint>>
function entrypoint_bug42326(): void {
  $request = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" xmlns:ns1="http://www.example.com/"><SOAP-ENV:Body><ns1:GetProductsRequest><time></time></ns1:GetProductsRequest></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;


  $soap_admin_classmap = dict['productDetailsType' => 'SOAP_productDetailsType',
                               'GetProductsRequest' => 'SOAP_GetProductsRequest',
                               'GetProductsResponse' => 'SOAP_GetProductsResponse'];

  $soap = new SoapServer(dirname(__FILE__).'/bug42326.wsdl', dict['classmap' => $soap_admin_classmap]);
  $soap->setClass('SOAP_Admin');
  ob_start();
  $soap->handle($request);
  ob_end_clean();
  echo "ok\n";
}
