<?hh # vim:ft=php

class FruitPublic
{
  public $title;

  public function __construct($title)
  {
    $this->title = $title;
  }

  public function __toString()
  {
    return $this->title;
  }
}

class FruitProtected
{
  protected $title;

  public function __construct($title)
  {
    $this->title = $title;
  }

  public function __toString()
  {
    return $this->title;
  }
}

function test_array($array, $which, $flags = 0)
{
  echo "===$which===\n";
  $it = new RecursiveArrayIterator($array, $flags);
  foreach (new RecursiveIteratorIterator($it) as $k => $fruit) {
    echo $k , ' => ', $fruit, "\n";
  }
}
<<__EntryPoint>>
function main_entry(): void {

  $array = darray[
    1 => darray[
      1 => darray[
        1 => 'apple',
      ],
      2 => darray[
        1 => 'grape',
      ],
    ],
  ];

  test_array($array, 'Default with array');

  $array = darray[
    1 => darray[
      1 => darray[
        1 => new FruitPublic('apple'),
      ],
      2 => darray[
        1 => new FruitPublic('grape'),
      ],
    ],
  ];

  test_array($array, 'Public Property');

  $array = darray[
    1 => darray[
      1 => darray[
        1 => new FruitProtected('apple'),
      ],
      2 => darray[
        1 => new FruitProtected('grape'),
      ],
    ],
  ];

  test_array($array, 'Protected Property');

  test_array($array, 'Public Property New', RecursiveArrayIterator::CHILD_ARRAYS_ONLY);
  test_array($array, 'Protected Property New', RecursiveArrayIterator::CHILD_ARRAYS_ONLY);
  echo "===DONE===\n";
}
