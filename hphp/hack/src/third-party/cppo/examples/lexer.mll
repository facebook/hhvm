(* Warning: ocamllex doesn't accept cppo directives
            within the rules section. *)
rule token = parse
    ['a'-'z']+  { `String (Lexing.lexeme lexbuf) }
{
#ifndef NOFOO
  let foo () = ()
#endif
}
