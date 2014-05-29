<?php
// Issue #2693

/*
           .
         /    \
       a1      a2
     / | \      |
    b1 b2 b3   b4
                |
               c1
*/

$arrayForTest = [
    'a1' => [
        'b1' => new StdClass(),
        'b2' => new StdClass(),
        'b3' => [],
    ],
    'a2' => [
        'b4' => [
            'c1' => new StdClass(),
        ]
    ]
];


class TestingFilterIterator extends RecursiveFilterIterator {
    public function accept()
    {
        return !($this->current() instanceof StdClass);
    }
}


$iterator = new RecursiveArrayIterator($arrayForTest);
$filter   = new TestingFilterIterator($iterator);
$final    = new RecursiveIteratorIterator($filter,
                                         RecursiveIteratorIterator::SELF_FIRST);

$keys = [];
foreach($final as $key => $value)
{
    $keys[] = $key;
}

var_dump($keys);
