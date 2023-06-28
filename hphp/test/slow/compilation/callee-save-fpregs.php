<?hh

class X {
  function __toString() :mixed{

    $a0 = (float)CompilationCalleeSaveFpregs::$g[0];
    $a1 = (float)CompilationCalleeSaveFpregs::$g[1];
    $a2 = (float)CompilationCalleeSaveFpregs::$g[2];
    $a3 = (float)CompilationCalleeSaveFpregs::$g[3];
    $a4 = (float)CompilationCalleeSaveFpregs::$g[4];
    $a5 = (float)CompilationCalleeSaveFpregs::$g[5];
    $a6 = (float)CompilationCalleeSaveFpregs::$g[6];
    $a7 = (float)CompilationCalleeSaveFpregs::$g[7];
    $a8 = (float)CompilationCalleeSaveFpregs::$g[8];
    $a9 = (float)CompilationCalleeSaveFpregs::$g[9];
    $a10 = (float)CompilationCalleeSaveFpregs::$g[10];
    $a11 = (float)CompilationCalleeSaveFpregs::$g[11];
    $a12 = (float)CompilationCalleeSaveFpregs::$g[12];

    $x = ($a0 + $a1) + ($a2 + $a3) + ($a4 + $a5) +
         ($a6 + $a7) + ($a8 + $a9) + ($a10 + $a11) + $a12;

    return (string)$x;
  }
}

function test($a, $x) :mixed{
  return HH\Lib\Legacy_FIXME\eq($a, $x);
}

abstract final class CompilationCalleeSaveFpregs {
  public static $g;
}
<<__EntryPoint>>
function entrypoint_calleesavefpregs(): void {

  CompilationCalleeSaveFpregs::$g = vec[0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];

  $x = vec[new X];
  for ($i = 0; $i < 5; $i++) {
    var_dump(test(vec["78"], $x));
  }
}
