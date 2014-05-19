<?php


class Sorter
{

    // Type of return value. 
    protected static $type = 0;

    // We always return 0 ($a and $b are equal).
    public static function doSort($a, $b) {
        switch (self::$type) {
            case 1: return 0;
            case 2: return 0.0;
            case 3: return '0';
        }
    }

    // Gets a sorted array.
    public function getSortedArray($returnType) {
        self::$type = $returnType;
        $sortedArray = array('a', 'b', 'c');
        uasort($sortedArray, 'Sorter::doSort');
        return $sortedArray;
    }

}

foreach (array(1, 2, 3) as $i) {
    var_dump(Sorter::getSortedArray($i));
}
