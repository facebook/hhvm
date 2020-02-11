<?hh
/* Prototype  : string http_build_query ( mixed $query_data [, string $numeric_prefix [, string $arg_separator [, int $enc_type = PHP_QUERY_RFC1738 ]]] )
 * Description: Generates a URL-encoded query string from the associative (or indexed) array provided. 
 * Source code: ext/standard/http.c
*/
<<__EntryPoint>> function main(): void {
$mDimensional = array(
  20, 
  5 => 13, 
  "9" => darray[
    1 => "val1", 
    3 => "val2", 
    "string" => "string"
  ],
  "name" => "homepage", 
  "page" => 10,
  "sort" => array(
    "desc", 
    "admin" => array(
      "admin1", 
      "admin2" => darray[
        "who" => "admin2", 
        2 => "test"
      ]
    )
  )
);

echo http_build_query($mDimensional) . PHP_EOL;
echo http_build_query($mDimensional, 'prefix_');
}
