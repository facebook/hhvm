<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

MemoizeKeyCountsFuncPhp::$do_echo = true;

<<__Memoize>> function func0() {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func0()\n";
  return json_encode([]);
}
<<__Memoize>> function func1($p1) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func1($p1)\n";
  return json_encode([$p1]);
}
<<__Memoize>> function func2($p1, $p2) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func2($p1, $p2)\n";
  return json_encode([$p1, $p2]);
}
<<__Memoize>> function func3($p1, $p2, $p3) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func3($p1, $p2, $p3)\n";
  return json_encode([$p1, $p2, $p3]);
}
<<__Memoize>> function func4($p1, $p2, $p3, $p4) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func4($p1, $p2, $p3, $p4)\n";
  return json_encode([$p1, $p2, $p3, $p4]);
}
<<__Memoize>> function func5($p1, $p2, $p3, $p4, $p5) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func5($p1, $p2, $p3, $p4, $p5)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5]);
}
<<__Memoize>> function func6($p1, $p2, $p3, $p4, $p5, $p6) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func6($p1, $p2, $p3, $p4, $p5, $p6)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6]);
}
<<__Memoize>> function func7($p1, $p2, $p3, $p4, $p5, $p6, $p7) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func7($p1, $p2, $p3, $p4, $p5, $p6, $p7)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7]);
}
<<__Memoize>> function func8($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func8($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8]);
}
<<__Memoize>> function func9($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func9($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9]);
}
<<__Memoize>> function func10($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func10($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10]);
}
<<__Memoize>> function func11($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func11($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11]);
}
<<__Memoize>> function func12($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11, $p12) {

  if (MemoizeKeyCountsFuncPhp::$do_echo) echo "func12($p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11, $p12)\n";
  return json_encode([$p1, $p2, $p3, $p4, $p5, $p6, $p7, $p8, $p9, $p10, $p11, $p12]);
}

function test() {


  var_dump(func0());
  var_dump(func1(1.1));
  var_dump(func2(1.1, 2.2));
  var_dump(func3(1.1, 2.2, 3.3));
  var_dump(func4(1.1, 2.2, 3.3, 4.4));
  var_dump(func5(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump(func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12));

  var_dump(func0());
  var_dump(func1(1.1));
  var_dump(func2(1.1, 2.2));
  var_dump(func3(1.1, 2.2, 3.3));
  var_dump(func4(1.1, 2.2, 3.3, 4.4));
  var_dump(func5(1.1, 2.2, 3.3, 4.4, 5.5));
  var_dump(func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12));

  var_dump(func0());
  var_dump(func1(1.19));
  var_dump(func2(1.19, 2.2));
  var_dump(func3(1.19, 2.2, 3.3));
  var_dump(func4(1.19, 2.2, 3.3, 4.4));
  var_dump(func5(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump(func6(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(func7(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(func8(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(func9(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(func10(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(func11(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(func12(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12));

  var_dump(func0());
  var_dump(func1(1.15));
  var_dump(func2(1.1, 2.25));
  var_dump(func3(1.1, 2.2, 3.35));
  var_dump(func4(1.1, 2.2, 3.3, 4.45));
  var_dump(func5(1.1, 2.2, 3.3, 4.4, 5.55));
  var_dump(func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.65));
  var_dump(func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75));
  var_dump(func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85));
  var_dump(func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95));
  var_dump(func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15));
  var_dump(func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115));
  var_dump(func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.125));

  MemoizeKeyCountsFuncPhp::$do_echo = false;
  for ($i = 0; $i < 30000; $i++) {
    func1(1.15+$i);
    func2(1.1, 2.25+$i);
    func3(1.1, 2.2, 3.35+$i);
    func4(1.1, 2.2, 3.3, 4.45+$i);
    func5(1.1, 2.2, 3.3, 4.4, 5.55+$i);
    func6(1.1, 2.2, 3.3, 4.4, 5.5, 6.65+$i);
    func7(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.75+$i);
    func8(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.85+$i);
    func9(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.95+$i);
    func10(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.15+$i);
    func11(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.115+$i);
    func12(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.125+$i);
  }

  MemoizeKeyCountsFuncPhp::$do_echo = true;
  var_dump(func0());
  var_dump(func1(1.19));
  var_dump(func2(1.19, 2.2));
  var_dump(func3(1.19, 2.2, 3.3));
  var_dump(func4(1.19, 2.2, 3.3, 4.4));
  var_dump(func5(1.19, 2.2, 3.3, 4.4, 5.5));
  var_dump(func6(1.19, 2.2, 3.3, 4.4, 5.5, 6.6));
  var_dump(func7(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7));
  var_dump(func8(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  var_dump(func9(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  var_dump(func10(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1));
  var_dump(func11(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11));
  var_dump(func12(1.19, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11, 12.12));

}
test();

abstract final class MemoizeKeyCountsFuncPhp {
  public static $do_echo;
}
