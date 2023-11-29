<?hh

function utf8StringToArray(string $s): vec<string> {
  $ret = vec[];
  $len = fb_utf8_strlen_deprecated($s);
  for ($i = 0; $i < $len; $i++) {
    $v = fb_utf8_substr($s, $i, 1);
    if ($v !== '') {
      $ret[] = $v;
    }
  }
  return $ret;
}

function test(string $s): vec<string> {
  $result = fb_utf8_decompose($s);
  $expect = utf8StringToArray($s);
  if ($result !== $expect) {
    $r = json_encode($result, JSON_FB_LOOSE);
    $e = json_encode($expect, JSON_FB_LOOSE);
    echo "Expected $e but got $r\n";
  }
  return $result;
}

// See utf8ize.php
<<__EntryPoint>>
function main_utf8_substr() :mixed{
  $INVALID_UTF_8_STRING = "\xe2\x82\x28";
  $SMILE = "\xf0\x9f\x98\x80";

  // Common cases
  echo json_encode(test(""), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("hon\xE7k"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("X"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("Hello"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("$SMILE"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("x$SMILE"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("{$SMILE}x"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("$SMILE$SMILE"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("xx{$SMILE}{$SMILE}zz"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("xx{$SMILE}yy{$SMILE}zz"), JSON_FB_LOOSE), PHP_EOL;

  // Invalid sequence
  echo json_encode(test($INVALID_UTF_8_STRING), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("x$INVALID_UTF_8_STRING"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("{$INVALID_UTF_8_STRING}x"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("$INVALID_UTF_8_STRING$INVALID_UTF_8_STRING"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("xx{$INVALID_UTF_8_STRING}{$INVALID_UTF_8_STRING}zz"), JSON_FB_LOOSE), PHP_EOL;
  echo json_encode(test("xx{$INVALID_UTF_8_STRING}yy{$INVALID_UTF_8_STRING}zz"), JSON_FB_LOOSE), PHP_EOL;
}
