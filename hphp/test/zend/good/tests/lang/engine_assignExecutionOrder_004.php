<?php

function i1() {
        echo "i1\n";
        return 1;
}

function i2() {
        echo "i2\n";
        return 1;
}

function i3() {
        echo "i3\n";
        return 3;
}

function i4() {

        LangEngineAssignexecutionorder004::$a = array(10, 11, 12, 13, 14);
        echo "i4\n";
        return 4;
}

LangEngineAssignexecutionorder004::$a = 0; // $a should not be indexable till the i4 has been executed
list(LangEngineAssignexecutionorder004::$a[i1()+i2()], , list(LangEngineAssignexecutionorder004::$a[i3()], LangEngineAssignexecutionorder004::$a[i4()]), LangEngineAssignexecutionorder004::$a[]) = array (0, 1, array(30, 40), 3, 4);

var_dump(LangEngineAssignexecutionorder004::$a);

abstract final class LangEngineAssignexecutionorder004 {
  public static $a;
}
