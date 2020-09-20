module Env = Full_fidelity_parser_env
module TestUtils = Full_fidelity_test_utils
module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (Syntax)
module VerifySC = VerifySmartConstructors.WithSyntax (Syntax)
module VerifyParser_ = Full_fidelity_parser.WithSyntax (Syntax)
module VerifyParser = VerifyParser_.WithSmartConstructors (VerifySC)

let verify_test_description =
  "This test is to verify that the order in which you call Make.* functions is the
same that in which you use the results. Eg. If you want to create a SyntaxList
with Missing and Token, make sure that you call Make.missing, then Make.token
and ultimately Make.list."

exception MoreThanOneElementInTheState of Syntax.t list

let verify ?(env = Env.default) text =
  let parser = VerifyParser.make env text in
  try
    let mode = Full_fidelity_parser.parse_mode text in
    let (parser, root, none_) = VerifyParser.parse_script parser in
    assert (none_ = None);
    let sc_state = VerifyParser.sc_state parser in
    if List.length sc_state > 1 then
      raise (MoreThanOneElementInTheState sc_state);
    let errors = VerifyParser.errors parser in
    SyntaxTree.create text root errors mode
  with
  | VerifySC.NotPhysicallyEquals (cons_name, stack, params, args) ->
    failwith
    @@ Printf.sprintf
         "%s\n
Argument lists are **PHYSICALLY** different in \"%s\" smart constructor call.
This means that there are two structurally equal (n1 = n2) but still
different (n1 != n2) nodes and you are passing one instead of the other.\n
STACK:\n%s\n
%s
BACKTRACE:\n%s"
         verify_test_description
         cons_name
         (TestUtils.tree_dump_list stack)
         (TestUtils.dump_diff params args)
         (Printexc.get_backtrace ())
  | VerifySC.NotEquals (cons_name, stack, params, args) ->
    failwith
    @@ Printf.sprintf
         "%s\n
Argument list mismatch in \"%s\" smart constructor call.\n
STACK:\n%s\n
%s
BACKTRACE:\n%s"
         verify_test_description
         cons_name
         (TestUtils.tree_dump_list stack)
         (TestUtils.dump_diff params args)
         (Printexc.get_backtrace ())
  | MoreThanOneElementInTheState state ->
    failwith
    @@ Printf.sprintf
         "%s\n
Smart constructor state contains more than 1 element after parsing.\n
STACK:\n%s\n"
         verify_test_description
         (TestUtils.tree_dump_list state)
