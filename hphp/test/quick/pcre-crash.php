<?hh

$response_text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
$patt = "/\\\\(?:(?:\\\\|u00[0-7])(*SKIP)(*FAIL)|u([0-9a-fA-F]{4}))/";
$callback = function ($m) { return html2txt("&#x$m[1];"); };
$response_text = preg_replace_callback($patt,
                                       $callback,
                                       $response_text);
var_dump($response_text);
