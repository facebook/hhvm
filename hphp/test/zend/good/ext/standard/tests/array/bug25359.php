<?hh

abstract final class ZendGoodExtStandardTestsArrayBug25359 {
  public static $data;
}

function does_not_work()
{
    ZendGoodExtStandardTestsArrayBug25359::$data = array('first', 'fifth', 'second', 'forth', 'third');
    $sort = array(1, 5, 2, 4, 3);
    $data = ZendGoodExtStandardTestsArrayBug25359::$data;
    array_multisort2(&$sort, &$data);
    ZendGoodExtStandardTestsArrayBug25359::$data = $data;

    var_dump(ZendGoodExtStandardTestsArrayBug25359::$data);
}
<<__EntryPoint>> function main(): void {
does_not_work();
}
