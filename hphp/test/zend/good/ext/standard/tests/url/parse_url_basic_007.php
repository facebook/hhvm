<?hh

/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components
 * Source code: ext/standard/url.c
 * Alias to functions:
 */

/*
 * Parse a load of URLs without specifying PHP_URL_PATH as the URL component
 */

<<__EntryPoint>>
function main_entry(): void {
  include_once(dirname(__FILE__) . '/urls.inc');

  foreach ($urls as $url) {
  	echo "--> $url   : ";
  	var_dump(parse_url($url, PHP_URL_PATH));
  }

  echo "Done";
}
