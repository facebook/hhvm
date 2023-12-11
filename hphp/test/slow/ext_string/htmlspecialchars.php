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

function test_htmlspecialchars_decode() :mixed{
  $str = "<p>this -&gt; &quot;</p>";
  VS(htmlspecialchars_decode($str), "<p>this -> \"</p>");

  VS(htmlspecialchars_decode("&lt;"), "<");
  VS(htmlspecialchars_decode("&nbsp;"), "&nbsp;");

  VS(htmlspecialchars_decode("&amp; &Eacute; &Alpha; &#039;"),
                               "& &Eacute; &Alpha; '");
}

function test_htmlspecialchars() :mixed{
  VS(htmlspecialchars("<a href='test'>Test</a>", ENT_QUOTES),
     "&lt;a href=&#039;test&#039;&gt;Test&lt;/a&gt;");

  VS(bin2hex(htmlspecialchars("\xA0", ENT_COMPAT)), "");
  VS(bin2hex(htmlspecialchars("\xc2\xA0", ENT_COMPAT, "")), "c2a0");
  VS(bin2hex(htmlspecialchars("\xc2\xA0", ENT_COMPAT, "UTF-8")), "c2a0");
  $zfoo = "\0foo";
  VS(htmlspecialchars($zfoo, ENT_COMPAT), $zfoo);
  VS(fb_htmlspecialchars($zfoo, ENT_COMPAT), $zfoo);

  VS(fb_htmlspecialchars("abcdef'\"{}@gz", ENT_QUOTES,
                           "", vec["z"]),
     "abcdef&#039;&quot;&#123;&#125;&#064;g&#122;");

  VS(fb_htmlspecialchars("abcdef'\"".u('\u00a1\uabcd'), ENT_FB_UTF8,
                           "", vec["d"]),
     "abc&#100;ef&#039;&quot;&#xa1;&#xabcd;");

  VS(fb_htmlspecialchars("abcdef'\"".u('\u00a1\uabcd'), ENT_FB_UTF8_ONLY,
                           "", vec["d"]),
     "abcdef'\"&#xa1;&#xabcd;");

  // The rest here expects RuntimeOption::Utf8izeReplace = true;
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

  VS(fb_htmlspecialchars($input, ENT_QUOTES, "UtF-8", vec[]),
     $sanitized);

  VS(fb_htmlspecialchars($input, ENT_FB_UTF8, "utf-8", vec[]),
     '&#xa1;&#xfffd;A'.
     '&#x561;&#xfffd;&#xfffd;'.
     '&#x3862;&#xfffd;&#xfffd;'.
     '&#x218a3;&#xfffd;A'.
     'hello&#xfffd;world'.
     '&#xfffd;'.
     '&#xfffd;'.
     '&#xfffd;');

  VS(fb_htmlspecialchars($sanitized, ENT_QUOTES, "", vec[]),
     $sanitized);

  VS(fb_htmlspecialchars($zfoo, ENT_COMPAT, "UTF-8"), u('\ufffd')."foo");
}


<<__EntryPoint>>
function main_htmlspecialchars() :mixed{
test_htmlspecialchars_decode();
test_htmlspecialchars();
}
