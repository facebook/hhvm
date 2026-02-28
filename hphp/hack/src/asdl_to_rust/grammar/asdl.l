%%
module "MODULE"
attributes "ATTRIBUTES"
version "VERSION"
[a-zA-Z_][a-zA-Z_0-9]* "IDENTIFIER"
, "COMMA"
\* "STAR"
\? "QUESTION_MARK"
= "EQUAL"
\|  "PIPE"
\( "LPAREN"
\) "RPAREN"
\{ "LBRACE"
\} "RBRACE"
"(?:\\\\|\\"|[^"\n])*" "STRING_LITERAL"
--[^\n]*$ ;
[ \t\n\r]+ ;
