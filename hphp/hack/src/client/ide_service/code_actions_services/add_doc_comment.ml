open Hh_prelude
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let edits_of_candidate ~path ~line_to_offset candidate : Code_action_types.edits
    =
  let (line_minus_1, character_plus_1) =
    Full_fidelity_positioned_syntax.leading_start_position candidate
  in
  let line = line_minus_1 + 1 in
  let character = character_plus_1 - 1 in
  let edit =
    let text = {|/**
 * ${0:}
 */
|} in
    let pos =
      let offset = line_to_offset line in
      let pos_start = (line, offset, offset + character) in
      Pos.make_from_lnum_bol_offset ~pos_file:path ~pos_start ~pos_end:pos_start
    in
    Code_action_types.{ pos; text }
  in
  Relative_path.Map.singleton path [edit]

let to_refactor ~line_to_offset ~path candidate =
  let edits = lazy (edits_of_candidate ~path ~line_to_offset candidate) in
  Code_action_types.Refactor.{ title = "Add doc comment"; edits }

let find_candidate pos source_text positioned_tree =
  let root = PositionedTree.root positioned_tree in
  let (line, start, _) = Pos.info_pos pos in
  let offset =
    Full_fidelity_source_text.position_to_offset source_text (line, start + 1)
  in
  let nodes = Full_fidelity_positioned_syntax.parentage root offset in
  let nodes_and_kinds =
    nodes
    |> List.bind ~f:(fun node ->
           Full_fidelity_positioned_syntax.leading_token node
           |> Option.map ~f:(fun token ->
                  (node, token.Full_fidelity_positioned_token.kind))
           |> Option.to_list)
  in
  let open Full_fidelity_token_kind in
  match nodes_and_kinds with
  | (_, Name)
    :: ( named_node,
         (Function | Class | Interface | Enum | Newtype | Type | Trait) )
    :: _ ->
    let has_doc_comment =
      Option.is_some @@ Docblock_finder.get_docblock named_node
    in
    Option.some_if (not has_doc_comment) named_node
  | (_, Name)
    :: ( _,
         ( EndOfFile | Abstract | Arraykey | As | Async | Attribute | Await
         | Backslash | Binary | Bool | Boolean | Break | Case | Catch | Category
         | Children | Classname | Clone | Concurrent | Const | Construct
         | Continue | Ctx | Darray | Default | Dict | Do | Double | Echo | Else
         | Empty | Endif | Eval | Exports | Extends | Fallthrough | Float | File
         | Final | Finally | For | Foreach | Global | If | Implements | Imports
         | Include | Include_once | Inout | Instanceof | Insteadof | Int
         | Integer | Is | Isset | Keyset | Lateinit | List | Match | Mixed
         | Module | Nameof | Namespace | New | Newctx | Noreturn | Num | Parent
         | Print | Private | Protected | Public | Real | Reify | Require
         | Require_once | Required | Resource | Return | Self | Shape | Static
         | String | Super | Switch | This | Throw | Try | Tuple | Unset | Upcast
         | Use | Using | Var | Varray | Vec | Void | With | Where | While
         | Yield | NullLiteral | LeftBracket | RightBracket | LeftParen
         | RightParen | LeftBrace | RightBrace | Dot | MinusGreaterThan
         | PlusPlus | MinusMinus | StarStar | Star | Plus | Minus | Tilde
         | Exclamation | Dollar | Slash | Percent | LessThanEqualGreaterThan
         | LessThanLessThan | GreaterThanGreaterThan | LessThan | GreaterThan
         | LessThanEqual | GreaterThanEqual | EqualEqual | EqualEqualEqual
         | ExclamationEqual | ExclamationEqualEqual | Carat | Bar | Ampersand
         | AmpersandAmpersand | BarBar | Question | QuestionAs | QuestionColon
         | QuestionQuestion | QuestionQuestionEqual | Colon | Semicolon | Equal
         | StarStarEqual | StarEqual | SlashEqual | PercentEqual | PlusEqual
         | MinusEqual | DotEqual | LessThanLessThanEqual
         | GreaterThanGreaterThanEqual | AmpersandEqual | CaratEqual | BarEqual
         | Comma | At | ColonColon | EqualGreaterThan | EqualEqualGreaterThan
         | QuestionMinusGreaterThan | DotDotDot | DollarDollar | BarGreaterThan
         | SlashGreaterThan | LessThanSlash | LessThanQuestion | Backtick | XHP
         | Hash | Readonly | Internal | Package | Let | ErrorToken | Name
         | Variable | DecimalLiteral | OctalLiteral | HexadecimalLiteral
         | BinaryLiteral | FloatingLiteral | SingleQuotedStringLiteral
         | DoubleQuotedStringLiteral | DoubleQuotedStringLiteralHead
         | StringLiteralBody | DoubleQuotedStringLiteralTail
         | HeredocStringLiteral | HeredocStringLiteralHead
         | HeredocStringLiteralTail | NowdocStringLiteral | BooleanLiteral
         | XHPCategoryName | XHPElementName | XHPClassName | XHPStringLiteral
         | XHPBody | XHPComment | Hashbang ) )
    :: _ ->
    None
  | _ -> None

let find ~entry pos ctx =
  let source_text = Ast_provider.compute_source_text ~entry in
  let line_to_offset line =
    Full_fidelity_source_text.position_to_offset source_text (line, 0)
  in
  let path = entry.Provider_context.path in
  let positioned_tree = Ast_provider.compute_cst ~ctx ~entry in
  find_candidate pos source_text positioned_tree
  |> Option.map ~f:(to_refactor ~path ~line_to_offset)
  |> Option.to_list
