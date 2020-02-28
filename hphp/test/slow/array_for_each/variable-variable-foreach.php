<?hh
ArrayForEachVariableVariableForeach::$rows=varray[1,2,3];


function fn1() {


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

function fn2() {
    $row = ArrayForEachVariableVariableForeach::$row;
    echo "row={$row}\n";
}

// ORIGINAL ARRAY
print_r(ArrayForEachVariableVariableForeach::$rows);

// SOME ITERATIONS
fn1();

// THE MALFORMED ARRAY
print_r(ArrayForEachVariableVariableForeach::$rows);

abstract final class ArrayForEachVariableVariableForeach {
  public static $rows;
  public static $row;
}
