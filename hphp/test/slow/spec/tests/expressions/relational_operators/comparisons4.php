<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

///*
// Numeric strings with all kinds of operands, swapping them over to make
// LHS/RHS order is irrelevent.

$oper1 = varray["10", "-5.1"];
$oper2 = varray[0, 100, -3.4, TRUE, FALSE, NULL, "", "123", "abc", varray[], varray[10,2.3]];

foreach ($oper1 as $e1)
{
    $p1 = var_export($e1, true);
    foreach ($oper2 as $e2)
    {
        $p2 = preg_replace('/\s+|,\n(?=\)$)/', '', var_export($e2, true));
        echo "{$p1} >        {$p2}  result: "; try { var_dump($e1 > $e2); } catch (Throwable $_) { E(); }
        echo "{$p2} <=       {$p1}  result: "; try { var_dump($e2 <= $e1); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "{$p1} >=       {$p2}  result: "; try { var_dump($e1 >= $e2); } catch (Throwable $_) { E(); }
        echo "{$p2} <        {$p1}  result: "; try { var_dump($e2 < $e1); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "{$p1} <        {$p2}  result: "; try { var_dump($e1 < $e2); } catch (Throwable $_) { E(); }
        echo "{$p2} >=       {$p1}  result: "; try { var_dump($e2 >= $e1); } catch (Throwable $_) { E(); }
        echo "---\n";
        echo "{$p1} <=       {$p2}  result: "; try { var_dump($e1 <= $e2); } catch (Throwable $_) { E(); }
        echo "{$p2} >        {$p1}  result: "; try { var_dump($e2 > $e1); } catch (Throwable $_) { E(); }
        echo "=======\n";
    }
    echo "-------------------------------------\n";
}
}

function E() { echo "cannot be compared\n"; }
