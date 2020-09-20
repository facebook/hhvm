<?hh // strict
$foo = varray['should_stay_on_same_line'];
$foo = varray[
  'should_remain_on_new_line',
];
$foo = varray['should_stay_on_same_line'];
$foo = varray[
  'should_remain_on_new_line',
];

$foo = darray['should_stay' => 'on_same_line'];
$foo = darray[
  'should_remain' => 'on_new_line',
];
$foo = darray['should_stay' => 'on_same_line'];
$foo = darray[
  'should_remain' => 'on_new_line',
];

$foo = Set { 'should_stay_on_same_line' };
$foo = Set {
  'should_remain_on_new_line',
};
$foo = Vector { 'should_stay_on_same_line' };
$foo = Vector {
  'should_remain_on_new_line',
};
$foo = Map { 'should_stay' => 'on_same_line' };
$foo = Map {
  'should_remain' => 'on_new_line',
};

$foo = keyset['should_stay_on_same_line'];
$foo = keyset[
  'should_remain_on_new_line',
];
$foo = vec['should_stay_on_same_line'];
$foo = vec[
  'should_remain_on_new_line',
];
$foo = dict['should_stay' => 'on_same_line'];
$foo = dict[
  'should_remain' => 'on_new_line',
];

// If there are no elements, there should not be a newline.
$foo = dict[
];

type Foo = shape('should_stay' => OnSameLine);
type Foo = shape(
  'should_remain' => OnNewLine,
);

type Foo = shape('should_stay' => OnSameLine, ...);
type Foo = shape(
  'should_remain' => OnNewLine,
  ...
);

foo(shape('should_stay' => 'on_same_line'));
foo(shape(
  'should_remain' => 'on_new_line',
));
