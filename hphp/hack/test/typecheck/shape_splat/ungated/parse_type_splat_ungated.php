<?hh
// No __EnableUnstableFeatures attribute — should error

type TBase = shape('x' => int);
type TExtended = shape(...TBase, 'y' => string);

function test(TExtended $s): void {}
