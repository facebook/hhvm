<?hh
/* Prototype  : int ezmlm_hash  ( string $addr  )
 * Description: Calculate the hash value needed by EZMLM.
 * Source code: ext/standard/mail.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ezmlm_hash() : basic functionality ***\n";

var_dump(ezmlm_hash(b"webmaster@somewhere.com"));
var_dump(ezmlm_hash(b"foo@somewhere.com"));
echo "===Done===";
}
