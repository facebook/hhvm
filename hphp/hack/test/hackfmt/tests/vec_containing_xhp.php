<?hh

$items = vec[
  <ui:glyph image="foo" />,
  <ui:glyph image="bar" />,
  <ui:glyph image="baz" />,
  'qux',
];

$x = vec[//
<p>x</p>];

$x = vec[<p>{1//
}</p>];

$x = vec[<p>{1//
}</p>, 2];
