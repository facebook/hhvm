<?hh
// No __EnableUnstableFeatures attribute — `absent` syntax should error.

type WithAbsent = shape('y' => int, absent 'x');
