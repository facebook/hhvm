<?hh


<<__EntryPoint>>
function main_bad_xmlreader() {
$reader = new XMLReader();
$reader->xml('<?xml version="1.0" encoding="UTF-8"?><ZohoAPIError><error code="API104" message="Invalid authorization header"/></ZohoAPIError>');
$reader->read();
var_dump($reader->attributes === null);
}
