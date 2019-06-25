<?hh
function crash()
{
    set_error_handler(function () {});
    $var = 1;
    trigger_error('error');
    $var2 = $var;
    $var3 = $var;
    trigger_error('error');
}
<<__EntryPoint>> function main(): void {
$items = new ArrayObject();

try { unset($items[0]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { unset($items[0][0]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
crash();
echo "Worked!\n";
}
