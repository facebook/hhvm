<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x != false, true); }

// Php doesn't support \u escapes.
function u($x) :mixed{ return json_decode("\"" . $x . "\""); }


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_utf8ize_replace() :mixed{
$input =
  u('\u00a1')."\xc2\x41".
  u('\u0561')."\xd5\xe0".
  u('\u3862')."\xe3\x80\xf0".
  "\xf0\xa1\xa2\xa3".
  "\xf0\xa1\xa2\x41".
  "hello\x80world".
  "\xed\xa0\x80".
  "\xe0\x80\xbc".
  "\xc2";

$tmp = $input;
fb_utf8ize(inout $tmp);
$sanitized = $tmp;

VS(fb_htmlspecialchars($input, ENT_QUOTES, "", vec[]), $sanitized);

VS(fb_htmlspecialchars($input, ENT_FB_UTF8, "", null),
   "&#xa1;A".
   "&#x561;".
   "&#x3862;".
   "&#x218a3;A".
   "helloworld");

VS(fb_htmlspecialchars($sanitized, ENT_QUOTES, "", vec[]),
   $sanitized);

$zfoo = "\0foo";
VS(fb_htmlspecialchars($zfoo, ENT_COMPAT, "UTF-8"), "foo");
}
