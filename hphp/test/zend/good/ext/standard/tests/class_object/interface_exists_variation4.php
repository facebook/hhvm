<?hh
/* Prototype  : bool interface_exists(string classname [, bool autoload])
 * Description: Checks if the class exists
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

<<__EntryPoint>> function main(): void {
HH\autoload_set_paths(
  dict[
    'class' => dict[
      'autointerface' => 'AutoInterface.inc',
    ],
  ],
  __DIR__.'/',
);

echo "*** Testing interface_exists() : test autoload default value ***\n";


var_dump(interface_exists("AutoInterface"));

echo "\nDONE\n";
}
