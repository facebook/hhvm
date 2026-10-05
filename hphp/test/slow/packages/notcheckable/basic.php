<?hh

<<__RequirePackage('closed')>>
function require_closed(): void {}

<<__SoftRequirePackage('closed')>>
function soft_require_closed(): void {}

function package_expression_closed(): void {
  if (package closed) {}
}

<<__EntryPoint>>
function main(): void {
  var_dump(package_exists('open'));
  // An undeclared name stays checkable, so a typo reports absent.
  var_dump(package_exists('neverdeclared'));

  $checks = dict[
    'package_exists' => () ==> package_exists('closed'),
    'package' => package_expression_closed<>,
    '__RequirePackage' => require_closed<>,
    '__SoftRequirePackage' => soft_require_closed<>,
  ];
  foreach ($checks as $construct => $check) {
    try {
      $check();
      echo $construct.": no exception\n";
    } catch (InvalidOperationException $e) {
      echo $construct.': '.$e->getMessage()."\n";
    }
  }
}
