<?hh

<<__EntryPoint>>
function main_bad_xmlreader() :mixed{
$reader = new XMLReader();
$reader->XML('<?xml version="1.0" encoding="UTF-8"?><ZohoAPIError><error code="API104" message="Invalid authorization header"/></ZohoAPIError>');
$reader->read();
  try {
    var_dump($reader->attributes === null);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}
