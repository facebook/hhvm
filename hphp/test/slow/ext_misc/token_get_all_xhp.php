<?php

$src = token_get_all('<?hh class :test{category %test}');
$expected = array(
  array(
    T_OPEN_TAG,
    '<?hh ',
    1,
  ),
  array(
    T_CLASS,
    'class',
    1,
  ),
  array(
    T_WHITESPACE,
    ' ',
    1,
  ),
  array(
    T_XHP_LABEL,
    ':test',
    1,
  ),
  '{',
  array(
    T_XHP_CATEGORY,
    'category',
    1,
  ),
  array(
    T_WHITESPACE,
    ' ',
    1,
  ),
  array(
    T_XHP_CATEGORY_LABEL,
    '%test',
    1,
  ),
  '}',
);

var_dump($src == $expected);
