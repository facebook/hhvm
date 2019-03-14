<?php

function does_not_work()
{


    ZendGoodExtStandardTestsArrayBug25359::$data = array('first', 'fifth', 'second', 'forth', 'third');
    $sort = array(1, 5, 2, 4, 3);
    array_multisort(&$sort, &ZendGoodExtStandardTestsArrayBug25359::$data);

    var_dump(ZendGoodExtStandardTestsArrayBug25359::$data);
}

does_not_work();


abstract final class ZendGoodExtStandardTestsArrayBug25359 {
  public static $data;
}
