<?hh
<<__EntryPoint>> function main(): void {
$obj = new ArrayObject(array('1st', 1, 2=>'3rd', '4th'=>4));

var_dump($obj->getArrayCopy());

echo "===isset===\n";
var_dump(isset($obj[0]));
var_dump(isset($obj[1]));
var_dump(isset($obj[2]));
var_dump(isset($obj['4th']));
var_dump(isset($obj['5th']));
var_dump(isset($obj[6]));

echo "===offsetGet===\n";
var_dump($obj[0]);
var_dump($obj[1]);
var_dump($obj[2]);
var_dump($obj['4th']);
try { var_dump($obj['5th']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump($obj[6]); } catch (Exception $e) { echo $e->getMessage()."\n"; }

echo "===offsetSet===\n";
echo "WRITE 1\n";
$obj[1] = 'Changed 1';
var_dump($obj[1]);
echo "WRITE 2\n";
$obj['4th'] = 'Changed 4th';
var_dump($obj['4th']);
echo "WRITE 3\n";
$obj['5th'] = 'Added 5th';
var_dump($obj['5th']);
echo "WRITE 4\n";
$obj[6] = 'Added 6';
var_dump($obj[6]);

var_dump($obj[0]);
var_dump($obj[2]);

$x = $obj[6] = 'changed 6';
var_dump($obj[6]);
var_dump($x);

echo "===unset===\n";
var_dump($obj->getArrayCopy());
unset($obj[2]);
unset($obj['4th']);
try { unset($obj[7]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { unset($obj['8th']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
var_dump($obj->getArrayCopy());

echo "===DONE===\n";
}
