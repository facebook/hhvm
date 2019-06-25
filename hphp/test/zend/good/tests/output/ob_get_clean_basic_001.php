<?hh
/* 
 * proto bool ob_get_clean(void)
 * Function is implemented in main/output.c
*/ 
<<__EntryPoint>> function main(): void {
var_dump(ob_get_clean());

ob_start();
echo "Hello World";
var_dump(ob_get_clean());
}
