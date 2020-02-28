<?hh

// default argument values; must be constants (or intrinsic function calls like array)

///*
function f1($p1 = 10, $p2 = 1.23, $p3 = TRUE, $p4 = NULL, $p5 = "abc", $p6 = varray[1,2,3,varray[]])
{
    echo "f1:\n";
    echo "\$p1: $p1, \$p2: $p2, \$p3: $p3, \$p4: $p4, \$p5: $p5, \$p6: $p6\n";
}
//*/
///*
// 2 default followed by one non-default; unusual, but permitted

function f2($p1 = 100, $p2 = 1.23, $p3)
{
    echo "f2:\n";
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}
//*/
///*
// 1 default followed by one non-default followed by 1 default; unusual, but permitted

function f3($p1 = 100, $p2, $p3 = "abc")
{
    echo "f3:\n";
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}
//*/
///*
// 1 non-default followed by two default; unusual, but permitted

function f4($p1, $p2 = 1.23, $p3 = "abc")
{
    echo "f4:\n";
    echo "\$p1: ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2: ".($p2 == NULL ? "NULL" : $p2).
        ", \$p3: ".($p3 == NULL ? "NULL" : $p3)."\n";
}
//*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  f1();
  f1(20);
  f1(10, TRUE);
  f1(NULL, 12, 1.234);
  f1(FALSE, 12e2, varray[99,-99], "abc");
  f1(9, 8, 7, 6, 5);
  f1(10, 20, 30, 40, 50, 60);
  f1(1, 2, 3, 4, 5, 6, 7);

  try { f2(); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { f2(10); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { f2(10, 20); } catch (Exception $e) { var_dump($e->getMessage()); }
  f2(10, 20, 30);

  try { f3(); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { f3(10); } catch (Exception $e) { var_dump($e->getMessage()); }
  f3(10, 20);
  f3(10, 20, 30);

  try { f4(); } catch (Exception $e) { var_dump($e->getMessage()); }
  try { f4(10); } catch (Exception $e) { var_dump($e->getMessage()); }
  f4(10, 20);
  f4(10, 20, 30);
}
