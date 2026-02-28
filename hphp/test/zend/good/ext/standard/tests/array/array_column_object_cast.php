<?hh
class ColumnKeyClass {
    function __toString() :mixed{ return 'first_name'; }
}

class IndexKeyClass {
    function __toString() :mixed{ return 'id'; }
}

class ValueClass {
    function __toString() :mixed{ return '2135'; }
}

<<__EntryPoint>> function main(): void {
$column_key = new ColumnKeyClass();
$index_key = new IndexKeyClass();
$value = new ValueClass();


// Array representing a possible record set returned from a database
$records = vec[
    dict[
        'id' => $value,
        'first_name' => 'John',
        'last_name' => 'XXX'
    ],
    dict[
        'id' => 3245,
        'first_name' => 'Sally',
        'last_name' => 'Smith'
    ],
];
$firstNames = array_column($records, $column_key, $index_key);
print_r($firstNames);
var_dump($column_key);
var_dump($index_key);
var_dump($value);
}
