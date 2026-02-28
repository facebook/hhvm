<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

///*
// checkout the type and value of the result

$a = 10 < 20;
var_dump($a);   // bool(true)
$a = 10 >= 20;
var_dump($a);   // bool(false)
$a = HH\Lib\Legacy_FIXME\lte(10, "xxx");
var_dump($a);   // bool(false)
$a = "zz" > "xx";
var_dump($a);   // bool(true)
echo "\n";
//*/

///*
// NULL operand with all kinds of operands, swapping them over to make
// LHS/RHS order is irrelevent.

$oper1 = vec[NULL];
$oper2 = vec[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", vec[], vec[10,2.3]];

foreach ($oper1 as $e1)
{
    $p1 = var_export($e1, true);
    foreach ($oper2 as $e2)
    {
        $p2 = preg_replace('/\s+|,\n(?=\)$)/', '', var_export($e2, true));
        echo "      {$p1} >        {$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gt($e1, $e2)); } catch (Throwable $_) { E(); }
        echo "      {$p1} >  (bool){$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gt($e1, (bool)$e2)); } catch (Throwable $_) { E(); }
        echo "      {$p2} <=       {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lte($e2, $e1)); } catch (Throwable $_) { E(); }
        echo "(bool){$p2} <=       {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lte((bool)$e2, $e1)); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "      {$p1} >=       {$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gte($e1, $e2)); } catch (Throwable $_) { E(); }
        echo "      {$p1} >= (bool){$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gte($e1, (bool)$e2)); } catch (Throwable $_) { E(); }
        echo "      {$p2} <        {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lt($e2, $e1)); } catch (Throwable $_) { E(); }
        echo "(bool){$p2} <        {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lt((bool)$e2, $e1)); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "      {$p1} <        {$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lt($e1, $e2)); } catch (Throwable $_) { E(); }
        echo "      {$p1} <  (bool){$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lt($e1, (bool)$e2)); } catch (Throwable $_) { E(); }
        echo "      {$p2} >=       {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gte($e2, $e1)); } catch (Throwable $_) { E(); }
        echo "(bool){$p2} >=       {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gte((bool)$e2, $e1)); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "      {$p1} <=       {$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lte($e1, $e2)); } catch (Throwable $_) { E(); }
        echo "      {$p1} <= (bool){$p2}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\lte($e1, (bool)$e2)); } catch (Throwable $_) { E(); }
        echo "      {$p2} >        {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gt($e2, $e1)); } catch (Throwable $_) { E(); }
        echo "(bool){$p2} >        {$p1}  result: "; try { var_dump(HH\Lib\Legacy_FIXME\gt((bool)$e2, $e1)); } catch (Throwable $_) { E(); }
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
}

function E() :mixed{ echo "cannot be compared\n"; }
