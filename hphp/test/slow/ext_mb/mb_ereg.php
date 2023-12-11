<?hh


<<__EntryPoint>>
function main_mb_ereg() :mixed{
var_dump(!mb_ereg_match("a", "some apples"));
var_dump(mb_ereg_match("a", "a kiwi"));
var_dump(mb_ereg_match(".*a", "some apples"));

$str = "This is a test";
var_dump(str_replace(" is", " was", $str));
var_dump(mb_ereg_replace("( )is", "\\1was", $str));
var_dump(mb_ereg_replace("(( )is)", "\\2was", $str));

$num = '4';
$str = "This string has four words.";
$str = mb_ereg_replace("four", $num, $str);
var_dump($str);

$test = "http://test.com/test";
$test = mb_ereg_replace("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]",
                         "<a href=\"\\0\">\\0</a>", $test);
var_dump($test);

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r === vec["Pr\xC3\x9C\xC3\x9D"."fung"]);
var_dump(mb_ereg_search_getpos());

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r == vec["Pr\xC3\x9C\xC3\x9D"."fung"]);

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r == vec["Pr\xC3\x9C\xC3\x9D"."fung"]);
var_dump(mb_ereg_search_pos());

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r === vec["Pr\xC3\x9C\xC3\x9D"."fung"]);
$r = mb_ereg_search_regs();    // get next result
var_dump($r);

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r === vec["Pr\xC3\x9C\xC3\x9D"."fung"]);
var_dump(mb_ereg_search_setpos(15));
$r = mb_ereg_search_regs();    // get next result
var_dump($r == vec["p\xC3\x9C"]);

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str);
$r = mb_ereg_search_regs("abc", "ms");
var_dump($r);

$str = "Pr\xC3\x9C\xC3\x9D"."fung abc p\xC3\x9C";
$reg = "\\w+";
mb_regex_encoding("UTF-8");
mb_ereg_search_init($str, $reg);
$r = mb_ereg_search();
$r = mb_ereg_search_getregs(); // get first result
var_dump($r === vec["Pr\xC3\x9C\xC3\x9D"."fung"]);

$date = "1973-04-30";
$regs = null;
mb_ereg("([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})", $date, inout $regs);
var_dump($regs[3]);
var_dump($regs[2]);
var_dump($regs[1]);
var_dump($regs[0]);

$pattern = "(>[^<]*)(suffix)";
$replacement = "\\1<span class=\"search\">\\2</span>";
$body = ">whateversuffix";
$body = mb_eregi_replace($pattern, $replacement, $body);
var_dump($body);

$pattern = "(>[^<]*)(suffix)";
$replacement = "\\1<span class=\"search\">\\2</span>";
$body = ">whateversuffix";
$body = mb_eregi_replace($pattern, $replacement, $body, "ip");
var_dump($body);

$str = "XYZ";
$matches = null;
var_dump(mb_eregi("z", $str, inout $matches));

$str = "XYZ";
$matches = vec[];
var_dump(mb_eregi("z", $str, inout $matches));
var_dump($matches);
}
