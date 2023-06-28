<?hh


/* patched HHVM will issue a warning about double mailheader */

<<__EntryPoint>>
function main_bug_68776() :mixed{
mail("test@example.com", "Subject", "Message", "Header-1:1\n\nHeader-2");
}
