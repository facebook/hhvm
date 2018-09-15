<?hh
// where, super are hack specific and case sensitive
FUNCTION FOO<T1 AS INT, T2 super T1, PROHIBITED>(
  T1 $_a,
  T2 $_b,
  PROHIBITED $_c
): VOID where PROHIBITED = T1 {
  ECHO "NO CASH VALUE\n";
}


<<__EntryPoint>>
function main_hack_generics() {
FOO(1, 2, 3);
}
