<?hh

enum E : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = self::FOO | self::BAR;
}
