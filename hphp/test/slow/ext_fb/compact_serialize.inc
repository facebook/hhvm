<?hh

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

  $array_tests = vec[];
  if ($v is KeyedContainer<_,_>) {
    $array_tests = vec[
      'is_array',
      'HH\\is_darray',
      'HH\\is_varray',
    ];
  }
  foreach ($array_tests as $t) {
    printf(
      "%s: %s\n",
      $t, var_export($t($v_), true),
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
    var_export($compact_unserialize_result ===  $v_, true),
    gettype($v_), gettype($compact_unserialize_result),
    var_export($ret === true, true),
  );
  foreach ($array_tests as $t) {
    printf(
      "%s: fb_cu(fb_s(type:%s)) === %s\n",
      $t,
      gettype($v_),
      var_export($t($compact_unserialize_result), true),
    );
  }
  echo "\n";
  $ret = null;
  $unserialize_result = fb_unserialize($s_, inout $ret);
  printf(
    "fb_u(fb_cs(x)) === x: %s\n"
    ."type(fb_u(fb_s(type:%s))) === %s\n"
    ."fbcu success: %s\n",
    var_export($unserialize_result === $v_, true),
    gettype($v_), gettype($unserialize_result),
    var_export($ret === true, true),
  );

  foreach ($array_tests as $t) {
    printf(
      "%s: fb_u(fb_cs(type:%s)) === %s\n",
      $t,
      gettype($v_),
      var_export($t($unserialize_result), true),
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
    varray[],
    darray[],

    varray[12345,"abc",0.1234],
    vec[12345,"abc",0.1234],
    darray[1 => 12345],
    dict[1 => 12345],
    darray[1 => 12345, "a" => 123124, "sdf" => 0.1234],
    dict[1 => 12345, "a" => 123124, "sdf" => 0.1234],

    varray[varray["a"]],
    vec[varray["a"]],
    varray[vec["a"]],
    vec[vec["a"]],

    varray[1, varray["a"]],
    vec[1, varray["a"]],
    varray[1, vec["a"]],
    vec[1, vec["a"]],

    varray[varray["a"]],
    varray[vec["a"]],
    vec[varray["a"]],

    varray[varray["a"], 1],
    varray[vec["a"], 1],
    vec[varray["a"], 1],
    vec[vec["a"], 1],

    varray[varray["a"], varray[1]],
    vec[varray["a"], varray[1]],
    varray[varray["a"], vec[1]],
    vec[vec["a"], varray[1]],
    varray[vec["a"], vec[1]],
    vec[vec["a"], vec[1]],

    varray[new stdclass(), new stdclass()],
    darray[42 => new stdclass(), 47 => new stdclass()],
  );

  // Test list-like darray/dict
  fb_cs_test(
    //k0=int,k1=int
    darray[0 => 'a', 1 => 'b'],
    dict[0 => 'a', 1 => 'b'],

    //k0=str,k1=str
    darray['0' => 'a', '1' => 'b'],
    dict['0' => 'a', '1' => 'b'],

    //k0=int,k1=str
    darray[0 => 'a', '1' => 'b'],
    dict[0 => 'a', '1' => 'b'],

    //k0=str,k1=int
    darray['0' => 'a', 1 => 'b'],
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
    darray[0 => "a", 1 => "b", 3 => "c"],
    dict[0 => "a", 1 => "b", 3 => "c"],
    dict['0' => "a", '1' => "b", '3' => "c"],
    dict['0' => "a", 1 => "b", '3' => "c"],
    darray[1 => "a", 2 => "b", 3 => "c"],
    dict[1 => "a", 2 => "b", 3 => "c"],
    darray[0 => "a", 2 => "b", 3 => "c"],
    dict[0 => "a", 2 => "b", 3 => "c"],
    dict['0' => "a", '2' => "b", '3' => "c"],
    darray[3 => "a"],
    dict[3 => "a"],
    darray[2 => 2, 1 => 1, 3 => 3],
    dict[2 => 2, 1 => 1, 3 => 3],
  );
  // Test for overflow (1ull << 63) - 1
  fb_cs_test(varray[9223372036854775807, 'a']);

  // Test each power of two, +/- 1 and the negatives of them
  // Test a single number and packed inside an array
  for ($i = 0; $i < 64; ++$i) {
    $n = 1 << $i;
    fb_cs_test($n);    fb_cs_test(varray[$n]);
    fb_cs_test($n-1);  fb_cs_test(varray[$n-1]);
    fb_cs_test($n+1);  fb_cs_test(varray[$n+1]);
    fb_cs_test(-$n);   fb_cs_test(varray[-$n]);
    fb_cs_test(-$n-1); fb_cs_test(varray[-$n-1]);
    fb_cs_test(-$n+1); fb_cs_test(varray[-$n+1]);
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
}
