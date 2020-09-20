<?hh
/* Prototype  : int getmypid  ( void  )
 * Description: Gets the current PHP process ID.
 * Source code: ext/standard/pageinfo.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "Simple testcase for getmypid() function\n";

var_dump(getmypid());

echo "Done\n";
}
