<?hh


<<__EntryPoint>>
function main_429() :mixed{
$a = dict[];
$a[0] = 1;
$a[01] = 2;
$a[007] = 3;
$a[010] = 4;
$a[0xa] = 5;
var_dump($a);
try { var_dump("$a[0]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[1]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[7]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[10]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[01]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[007]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[08]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump("$a[0xa]"); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
