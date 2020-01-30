<?hh


class TestingFilterIterator extends RecursiveFilterIterator {
    public function accept()
    {
        return !($this->current() is StdClass);
    }
}

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

<<__EntryPoint>>
function main_level_recursive_filter_iterator() {
$arrayForTest = darray[
    'a1' => darray[
        'b1' => new StdClass(),
        'b2' => new StdClass(),
        'b3' => varray[],
    ],
    'a2' => darray[
        'b4' => darray[
            'c1' => new StdClass(),
        ]
    ]
];


$iterator = new RecursiveArrayIterator($arrayForTest);
$filter   = new TestingFilterIterator($iterator);
$final    = new RecursiveIteratorIterator($filter,
                                         RecursiveIteratorIterator::SELF_FIRST);

$keys = varray[];
foreach($final as $key => $value)
{
    $keys[] = $key;
}

var_dump($keys);
}
