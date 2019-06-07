<?hh


<<__EntryPoint>>
function main_unset_error() {
$ar = array('a'=>0, 1, 2, 3);
$ar = new ArrayObject($ar);

unset($ar['a']);
try { unset($ar[12]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { unset($ar['c']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { $ar->offsetUnset('c'); } catch (Exception $e) { echo $e->getMessage()."\n"; }


$obj = new stdClass();
$obj->one = 1;
$obj->two = 2;
$ar = new ArrayObject($obj);

unset($ar['one']);
try { unset($ar[12]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { unset($ar['c']); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { $ar->offsetUnset('c'); } catch (Exception $e) { echo $e->getMessage()."\n"; }
}
