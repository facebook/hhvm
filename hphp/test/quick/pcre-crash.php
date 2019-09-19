<?hh
<<__EntryPoint>> function main(): void {
$response_text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
$patt = "/\\\\(?:(?:\\\\|u00[0-7])(*SKIP)(*FAIL)|u([0-9a-fA-F]{4}))/";
$callback = function ($m) { return html2txt("&#x$m[1];"); };
$count = -1;
$response_text = preg_replace_callback($patt,
                                       $callback,
                                       $response_text,
                                       -1,
                                       inout $count);
var_dump($response_text);
}
