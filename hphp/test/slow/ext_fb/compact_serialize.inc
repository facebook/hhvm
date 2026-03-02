<?hh

function darray_recursive($x) {
  if (!HH\is_php_array($x)) {
    return $x;
  }
  $result = dict[];
  foreach ($x as $k => $v) {
    $result[$k] = darray_recursive($v);
  }
  return $result;
}

function hexdump(string $s): string {
  $out = "\n";
  for ($i=0; $i < strlen($s); ++$i) {
    $out .= sprintf('%02X', ord($s[$i]));
    $c = $i + 1;
    if ($c % 8) {
      $out .= ' ';
    } else {
      $out .= ($c % 16 === 0) ? "\n" : "  ";
    }
  }
  return $out;
}


function fb_cs_test(...$vv) {
  foreach ($vv as $v) {
    fb_cs_test_impl($v);
  }
}

function fb_cs_test_impl($v) {
  printf("\n\n====\nx = %s\n", var_export($v, true));

  $ret = null;
  $v_ = $v;

  $array_tests = dict[];
  if ($v is KeyedContainer<_,_>) {
    $array_tests = dict[
      'HH\\is_php_array' => $x ==> HH\is_php_array($x),
      'HH\\is_darray' => $x ==> HH\is_darray($x),
      'HH\\is_varray' => $x ==> HH\is_varray($x),
    ];
  }
  foreach ($array_tests as $name => $fn) {
    printf(
      "%s: %s\n",
      $name, var_export($fn($v_), true),
    );
  }
  echo "\n";
  $s_ = fb_compact_serialize($v_);
  echo "serialize output is string: " . var_export($s_ is string, true)."\n";
  var_dump(hexdump($s_));
  $ss_ = $s_;
  var_dump((bool)($ss_ ?? false));
  /* check high bit of first character always set */
  var_dump(preg_match("/^[\x80-\xff]/", $ss_));
  $err = null;
  $compact_unserialize_result = fb_compact_unserialize($s_, inout $ret, inout $err);
  printf(
    "fb_cu(fb_s(%s)) === %s: %s\n"
    ."type(fb_cu(fb_s(%s))) === %s\n"
    ."fbcu success: %s\n",
    var_export($v_, true), var_export($compact_unserialize_result, true),
    var_export($compact_unserialize_result === darray_recursive($v_), true),
    gettype($v_), gettype($compact_unserialize_result),
    var_export($ret === true, true),
  );
  foreach ($array_tests as $name => $fn) {
    printf(
      "%s: fb_cu(fb_s(type:%s)) === %s\n",
      $name,
      gettype($v_),
      var_export($fn($compact_unserialize_result), true),
    );
  }
  echo "\n";
  $ret = null;
  $unserialize_result = fb_unserialize($s_, inout $ret);
  printf(
    "fb_u(fb_cs(x)) === x: %s\n"
    ."type(fb_u(fb_s(type:%s))) === %s\n"
    ."fbcu success: %s\n",
    var_export($unserialize_result === darray_recursive($v_), true),
    gettype($v_), gettype($unserialize_result),
    var_export($ret === true, true),
  );

  foreach ($array_tests as $name => $fn) {
    printf(
      "%s: fb_u(fb_cs(type:%s)) === %s\n",
      $name,
      gettype($v_),
      var_export($fn($unserialize_result), true),
    );
  }
  echo "\n";
}

function fb_cs_entrypoint(): void {
  fb_cs_test(null);
  fb_cs_test(true);
  fb_cs_test(false);
  fb_cs_test(1234.5678);
  fb_cs_test("");
  fb_cs_test("a");
  fb_cs_test("\0");
  fb_cs_test("\0 a");
  fb_cs_test("0123012301230123");
  fb_cs_test("0123012301230123a");
  fb_cs_test("012301230123012");

  fb_cs_test(
    HH\array_mark_legacy(vec[]),
    HH\array_mark_legacy(dict[]),

    HH\array_mark_legacy(vec[12345,"abc",0.1234]),
    vec[12345,"abc",0.1234],
    HH\array_mark_legacy(dict[1 => 12345]),
    dict[1 => 12345],
    HH\array_mark_legacy(dict[1 => 12345, "a" => 123124, "sdf" => 0.1234]),
    dict[1 => 12345, "a" => 123124, "sdf" => 0.1234],

    HH\array_mark_legacy_recursive(vec[vec["a"]]),
    vec[HH\array_mark_legacy(vec["a"])],
    HH\array_mark_legacy(vec[vec["a"]]),
    vec[vec["a"]],

    HH\array_mark_legacy_recursive(vec[1, vec["a"]]),
    vec[1, HH\array_mark_legacy(vec["a"])],
    HH\array_mark_legacy(vec[1, vec["a"]]),
    vec[1, vec["a"]],

    HH\array_mark_legacy_recursive(vec[vec["a"]]),
    HH\array_mark_legacy(vec[vec["a"]]),
    vec[HH\array_mark_legacy(vec["a"])],

    HH\array_mark_legacy_recursive(vec[vec["a"], 1]),
    HH\array_mark_legacy(vec[vec["a"], 1]),
    vec[HH\array_mark_legacy(vec["a"]), 1],
    vec[vec["a"], 1],

    HH\array_mark_legacy_recursive(vec[vec["a"], vec[1]]),
    vec[HH\array_mark_legacy(vec["a"]), HH\array_mark_legacy(vec[1])],
    HH\array_mark_legacy(vec[HH\array_mark_legacy(vec["a"]), vec[1]]),
    vec[vec["a"], HH\array_mark_legacy(vec[1])],
    HH\array_mark_legacy(vec[vec["a"], vec[1]]),
    vec[vec["a"], vec[1]],

    HH\array_mark_legacy(vec[new stdClass(), new stdClass()]),
    HH\array_mark_legacy(dict[42 => new stdClass(), 47 => new stdClass()]),
  );

  // Test list-like darray/dict
  fb_cs_test(
    //k0=int,k1=int
    HH\array_mark_legacy(dict[0 => 'a', 1 => 'b']),
    dict[0 => 'a', 1 => 'b'],

    //k0=str,k1=str
    HH\array_mark_legacy(dict['0' => 'a', '1' => 'b']),
    dict['0' => 'a', '1' => 'b'],

    //k0=int,k1=str
    HH\array_mark_legacy(dict[0 => 'a', '1' => 'b']),
    dict[0 => 'a', '1' => 'b'],

    //k0=str,k1=int
    HH\array_mark_legacy(dict['0' => 'a', 1 => 'b']),
    dict['0' => 'a', 1 => 'b'],

    // keysets that are list like
    keyset[0, 1],
    keyset['0', 1],
    keyset[0, '1'],
    keyset['0', '1'],
    keyset[-1, 0],
    keyset['-1', 0],
  );

  // Test skips
  fb_cs_test(
    HH\array_mark_legacy(dict[0 => "a", 1 => "b", 3 => "c"]),
    dict[0 => "a", 1 => "b", 3 => "c"],
    dict['0' => "a", '1' => "b", '3' => "c"],
    dict['0' => "a", 1 => "b", '3' => "c"],
    HH\array_mark_legacy(dict[1 => "a", 2 => "b", 3 => "c"]),
    dict[1 => "a", 2 => "b", 3 => "c"],
    HH\array_mark_legacy(dict[0 => "a", 2 => "b", 3 => "c"]),
    dict[0 => "a", 2 => "b", 3 => "c"],
    dict['0' => "a", '2' => "b", '3' => "c"],
    HH\array_mark_legacy(dict[3 => "a"]),
    dict[3 => "a"],
    HH\array_mark_legacy(dict[2 => 2, 1 => 1, 3 => 3]),
    dict[2 => 2, 1 => 1, 3 => 3],
  );
  // Test for overflow (1ull << 63) - 1
  fb_cs_test(vec[9223372036854775807, 'a']);

  // Test each power of two, +/- 1 and the negatives of them
  // Test a single number and packed inside an array
  for ($i = 0; $i < 64; ++$i) {
    $n = 1 << $i;
    fb_cs_test($n);    fb_cs_test(vec[$n]);
    fb_cs_test($n-1);  fb_cs_test(vec[$n-1]);
    fb_cs_test($n+1);  fb_cs_test(vec[$n+1]);
    fb_cs_test(-$n);   fb_cs_test(vec[-$n]);
    fb_cs_test(-$n-1); fb_cs_test(vec[-$n-1]);
    fb_cs_test(-$n+1); fb_cs_test(vec[-$n+1]);
  }

  echo "====\n";
  echo "==== testing vector code\n";

  // Test vector code (PHP can't create those, but they might come form
  // C++ code in serialized strings)
  $s = "\xfe\x01\x02\x03\xfc";  // VECTOR, 1, 2, 3, STOP
  $ret = null;
  $err = null;
  var_dump(fb_compact_unserialize($s, inout $ret, inout $err));

  // Demonstrate vector deserialize issue.
  // This should be illegal and result in an error.
  $s = "\xfe\x01\xfd\xfd\x03\xfc";  // VECTOR, 1, SKIP, SKIP, 2, STOP
  $ret = null;
  var_dump(fb_compact_unserialize($s, inout $ret, inout $err));
  var_dump($ret);
  var_dump($err);

  echo "====\n";
  echo "==== testing compact keyset serialization\n";

  // Test compact keyset encoding with FB_COMPACT_SERIALIZE_KEYSETS option
  $keysets = vec[
    keyset[],
    keyset[1],
    keyset['a'],
    keyset[1, 2, 3],
    keyset['a', 'b', 'c'],
    keyset[1, 'a', 2, 'b'],
  ];

  foreach ($keysets as $ks) {
    echo "\n";
    printf("keyset = %s\n", var_export($ks, true));

    // Legacy encoding (no flag): MAP with key=key pairs
    $legacy = fb_compact_serialize($ks);
    printf("legacy hex: %s\n", hexdump($legacy));

    // Compact encoding (with flag): EXT_KEYSET with values only
    $compact = fb_compact_serialize($ks, FB_COMPACT_SERIALIZE_KEYSETS);
    printf("compact hex: %s\n", hexdump($compact));

    // Size comparison
    $legacy_len = strlen($legacy as string);
    $compact_len = strlen($compact as string);
    printf("legacy size: %d, compact size: %d\n", $legacy_len, $compact_len);

    // Round-trip the compact encoding
    $ret = null;
    $err = null;
    $result = fb_compact_unserialize($compact, inout $ret, inout $err);
    printf("round-trip success: %s\n", var_export($ret === true, true));
    printf("round-trip result === original: %s\n",
      var_export($result === $ks, true));
    printf("result is keyset: %s\n", var_export($result is keyset<_>, true));
  }

  echo "\n==== testing compact keyset deserialization from raw bytes\n";

  // Hand-crafted EXT_KEYSET: 0xFF 0x00 <values...> 0xFC
  // EXT=0xFF, EXT_KEYSET=0x00, 1, 2, 3, STOP=0xFC
  $s = "\xff\x00\x01\x02\x03\xfc";
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("raw keyset [1,2,3]: %s\n", var_export($result, true));
  printf("is keyset: %s\n", var_export($result is keyset<_>, true));
  printf("success: %s\n", var_export($ret === true, true));

  // Empty keyset: 0xFF 0x00 0xFC
  $s = "\xff\x00\xfc";
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("raw empty keyset: %s\n", var_export($result, true));
  printf("is keyset: %s\n", var_export($result is keyset<_>, true));
  printf("success: %s\n", var_export($ret === true, true));

  // String keyset: 0xFF 0x00 STRING_1 'x' STRING_1 'y' STOP
  $s = "\xff\x00\xf8x\xf8y\xfc";
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("raw keyset ['x','y']: %s\n", var_export($result, true));
  printf("is keyset: %s\n", var_export($result is keyset<_>, true));
  printf("success: %s\n", var_export($ret === true, true));

  // Unknown extension sub-type should fail
  $s = "\xff\x02\xfc";  // EXT, sub-type 2 (unknown), STOP
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("unknown ext sub-type: success=%s, errcode=%s\n",
    var_export($ret, true), var_export($err, true));

  // Test keyset nested inside vec/dict with the flag
  echo "\n==== testing nested keysets\n";
  $nested = vec[keyset[1, 2], dict['k' => keyset['a']]];
  $s = fb_compact_serialize($nested, FB_COMPACT_SERIALIZE_KEYSETS);
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("nested round-trip success: %s\n", var_export($ret === true, true));
  printf("result[0] is keyset: %s\n",
    var_export($result[0] is keyset<_>, true));
  printf("result[0]: %s\n", var_export($result[0], true));
  printf("result[1]['k'] is keyset: %s\n",
    var_export($result[1]['k'] is keyset<_>, true));
  printf("result[1]['k']: %s\n", var_export($result[1]['k'], true));

  // ===== keepClasses tests (FB_COMPACT_SERIALIZE_KEEP_CLASSES) =====

  echo "\n====\n";
  echo "==== testing compact keepClasses serialization\n";

  // Test keepClasses with FB_COMPACT_SERIALIZE_KEEP_CLASSES option
  $classes = vec[
    HH\classname_from_string_unsafe('MyClass'),
    HH\classname_from_string_unsafe('Some\Namespaced\Class'),
    HH\classname_from_string_unsafe('A'),
  ];

  foreach ($classes as $cls) {
    echo "\n";
    printf("class = %s\n", $cls as string);

    // Without flag: serialized as plain string
    $plain = fb_compact_serialize($cls);
    printf("plain hex: %s\n", hexdump($plain));

    // With flag: serialized with EXT_CLASS prefix
    $kept = fb_compact_serialize($cls, FB_COMPACT_SERIALIZE_KEEP_CLASSES);
    printf("kept hex: %s\n", hexdump($kept));

    // Round-trip the keepClasses encoding
    $ret = null;
    $err = null;
    $result = fb_compact_unserialize($kept, inout $ret, inout $err);
    printf("round-trip success: %s\n", var_export($ret === true, true));
    printf("result is class: %s\n", var_export($result is classname<mixed>, true));
    printf("result as string: %s\n", $result as string);
  }

  // Test keepClasses inside a container
  echo "\n";
  $container = dict[
    'cls' => HH\classname_from_string_unsafe('Foo'),
    'val' => 42,
  ];
  $opts = FB_COMPACT_SERIALIZE_KEEP_CLASSES;
  $s = fb_compact_serialize($container, $opts);
  printf("container hex: %s\n", hexdump($s));
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("container round-trip success: %s\n", var_export($ret === true, true));
  printf("cls is class: %s\n",
    var_export($result['cls'] is classname<mixed>, true));
  printf("cls value: %s\n", $result['cls'] as string);
  printf("val value: %d\n", $result['val']);

  // Test keepClasses without the flag produces a plain string on unserialize
  echo "\n";
  $cls = HH\classname_from_string_unsafe('Bar');
  $plain = fb_compact_serialize($cls);
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($plain, inout $ret, inout $err);
  printf("without flag, result is string: %s\n",
    var_export($result is string, true));
  printf("without flag, result is class: %s\n",
    var_export($result is classname<mixed>, true));
  printf("without flag, value: %s\n", $result as string);

  // Test deserialization of hand-crafted EXT_CLASS bytes
  echo "\n==== testing keepClasses deserialization from raw bytes\n";
  $s = "\xff\x01\xf8X";  // EXT, EXT_CLASS, STRING_1, 'X'
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("raw class 'X': %s\n", $result as string);
  printf("is class: %s\n", var_export($result is classname<mixed>, true));
  printf("success: %s\n", var_export($ret === true, true));

  // Test combined flags: KEYSETS | KEEP_CLASSES
  echo "\n==== testing combined flags\n";
  $combined_opts = FB_COMPACT_SERIALIZE_KEYSETS
    | FB_COMPACT_SERIALIZE_KEEP_CLASSES;
  $mixed_container = dict[
    'ks' => keyset['x', 'y'],
    'cls' => HH\classname_from_string_unsafe('Baz'),
    'num' => 7,
  ];
  $s = fb_compact_serialize($mixed_container, $combined_opts);
  printf("combined hex: %s\n", hexdump($s));
  $ret = null;
  $err = null;
  $result = fb_compact_unserialize($s, inout $ret, inout $err);
  printf("combined round-trip success: %s\n", var_export($ret === true, true));
  printf("ks is keyset: %s\n",
    var_export($result['ks'] is keyset<_>, true));
  printf("ks value: %s\n", var_export($result['ks'], true));
  printf("cls is class: %s\n",
    var_export($result['cls'] is classname<mixed>, true));
  printf("cls value: %s\n", $result['cls'] as string);
  printf("num value: %d\n", $result['num']);
}
