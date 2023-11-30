open Linting_visitors
open Nast
module Nast = Aast

type conditionKind =
  | CKString
  | CKLiteral of bool
  | CKUnaryNot of bool
  | CKUnknown

(* checks if condition consists only of literal values and unary not operator *)
class condition_visitor =
  object
    inherit [conditionKind] Visitor_DEPRECATED.visitor as parent

    method! on_expr_ acc e =
      match e with
      | Nast.String _ -> CKString
      | Nast.True -> CKLiteral true
      | Nast.False -> CKLiteral false
      | Nast.Unop (Ast_defs.Unot, e') -> begin
        match parent#on_expr acc e' with
        | CKLiteral v -> CKUnaryNot v
        | CKUnaryNot v -> CKUnaryNot (not v)
        | e -> e
      end
      | _ -> CKUnknown
  end
  [@alert "-deprecated"]

module VisitorFunctor (Parent : BodyVisitorModule) : BodyVisitorModule = struct
  class visitor lint_env =
    object
      inherit Parent.visitor lint_env as parent

      method! on_if _ (ty, p, e) b1 b2 =
        let is_invariant_violation_call =
          (* invariant() is not actually a function - both the typechecker and
           * the runtime transform it into an if statement with a call to
           * invariant_violation() inside.
           *)
          match b1 with
          | [
           ( _,
             Nast.Expr
               ( _,
                 _,
                 Nast.(
                   Call
                     {
                       func = (_, _, Nast.Id (_, "\\HH\\invariant_violation"));
                       _;
                     }) ) );
          ] ->
            true
          | _ -> false
        in
        let err_msg =
          match (new condition_visitor)#on_expr CKUnknown (ty, p, e) with
          | CKUnknown -> None
          | CKString ->
            let err_msg =
              if is_invariant_violation_call then
                "invariant('error message') will never or always crash. Did you mean invariant_violation()?"
              else
                "Putting a string literal in the condition of an if statement guarantees it will always or never trigger."
            in
            Some err_msg
          | CKUnaryNot true when is_invariant_violation_call ->
            let err_msg =
              "Your expression always evaluates to true. Because of this, invariant() will never throw an exception here. If this is the behavior you want simply remove or comment out the line."
            in
            Some err_msg
          | CKUnaryNot false when is_invariant_violation_call ->
            let err_msg =
              "Your expression always evaluates to false. Because of this, invariant() will always throw an exception here. If this is the behavior you want, please use invariant_violation() explicitly instead."
            in
            Some err_msg
          | _ -> None
        in
        begin
          match err_msg with
          | Some err_msg -> Lints_errors.if_literal p err_msg
          | None -> ()
        end;
        parent#on_if () (ty, p, e) b1 b2
    end
end

let go = lint_all_bodies (module VisitorFunctor)
