<?hh


function fn1() :mixed{


    $a="rows";
    $b="row";

    // TYPICAL FOREACH
    foreach(ArrayForEachVariableVariableForeach::$rows as ArrayForEachVariableVariableForeach::$row) {
        fn2();
    }

    // THE MALFORMED ARRAY
    foreach(ArrayForEachVariableVariableForeach::$rows as ArrayForEachVariableVariableForeach::$row) {
        fn2();
    }
}

function fn2() :mixed{
    $row = ArrayForEachVariableVariableForeach::$row;
    echo "row={$row}\n";
}

abstract final class ArrayForEachVariableVariableForeach {
  public static $rows;
  public static $row;
}
<<__EntryPoint>>
function entrypoint_variablevariableforeach(): void {
  ArrayForEachVariableVariableForeach::$rows=vec[1,2,3];

  // ORIGINAL ARRAY
  print_r(ArrayForEachVariableVariableForeach::$rows);

  // SOME ITERATIONS
  fn1();

  // THE MALFORMED ARRAY
  print_r(ArrayForEachVariableVariableForeach::$rows);
}
