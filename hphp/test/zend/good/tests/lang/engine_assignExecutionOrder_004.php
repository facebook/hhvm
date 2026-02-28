<?hh

function i1() :mixed{
        echo "i1\n";
        return 1;
}

function i2() :mixed{
        echo "i2\n";
        return 1;
}

function i3() :mixed{
        echo "i3\n";
        return 3;
}

function i4() :mixed{

        LangEngineAssignexecutionorder004::$a = vec[10, 11, 12, 13, 14];
        echo "i4\n";
        return 4;
}

abstract final class LangEngineAssignexecutionorder004 {
  public static $a;
}

<<__EntryPoint>> function main(): void {
LangEngineAssignexecutionorder004::$a = 0; // $a should not be indexable till the i4 has been executed
list(LangEngineAssignexecutionorder004::$a[i1()+i2()], , list(LangEngineAssignexecutionorder004::$a[i3()], LangEngineAssignexecutionorder004::$a[i4()]), LangEngineAssignexecutionorder004::$a[]) = vec[0, 1, vec[30, 40], 3, 4];

var_dump(LangEngineAssignexecutionorder004::$a);
}
