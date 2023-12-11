<?hh

// Format argument as type Object
class testObject
{
}
<<__EntryPoint>>
function entrypoint_get_headers_error_002(): void {
  /* Prototype  : proto array get_headers(string url[, int format])
   * Description: Fetches all the headers sent by the server in response to a HTTP request
   * Source code: ext/standard/url.c
   * Alias to functions:
   */

  echo "*** Testing get_headers() : error conditions ***\n";
  $url = 'http://php.net';

  // Format argument as type String
  echo "\n-- Testing get_headers() function with format argument as type string --\n";
  try { var_dump( get_headers($url, "#PHPTestFest2009 Norway") ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  // Format argument as type Array
  echo "\n-- Testing get_headers() function with format argument as type array --\n";
  try { var_dump( get_headers($url, vec[]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  $object = new testObject();
  echo "\n-- Testing get_headers() function with format argument as type object --\n";
  try { var_dump( get_headers($url, $object) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


  echo "Done";
}
