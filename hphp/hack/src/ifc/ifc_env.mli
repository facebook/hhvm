open Ifc_types

(* - Read-only environments (renv) - *)

(* Creates a read-only environment sufficient to call functions
   from ifc_lift.ml *)
val new_renv : Ifc_scope.t -> decl_env -> Tast.saved_env -> proto_renv

(* Prepares a read-only environment to type-check a function *)
val prep_renv : proto_renv -> ptype option -> ptype -> ptype -> policy -> renv

val new_policy_var : 'a renv_ -> string -> policy

(* - Read/write environments (env) - *)

(* To provide a safe API for environments that prevents
   programming errors, the env type is abstract and has
   two phantom parameters *)
type yes

and no

(* The first parameter is yes iff the environment has meaningful
   type information about the local variables.
   The second parameter is yes iff the environment can be used
   to register that an exception might be thrown (using the
   throw function below). *)
type ('has_locals, 'can_throw) env

(* blank_env, stmt_env, and expr_env are handy aliases; a
   blank_env cannot do much, stmt_env and expr_env are used
   to typecheck statements and expressions, respecitively *)
type blank_env = (no, no) env

type stmt_env = (yes, no) env

type expr_env = (yes, yes) env

val empty_env : blank_env

(* prep_stmt turns a blank_env into a stmt_env using a continuation
   that contains the control flow dependencies and the type of local
   variables *)
val prep_stmt : blank_env -> cont -> stmt_env

val prep_expr : stmt_env -> expr_env

val close_expr : expr_env -> stmt_env * cont KMap.t

(* close_stmt is used to finish the analysis of a statement; if
   the 'merge' outcome is provided it is merged in the final
   outcome.
   The Typing_cont_key.t parameter indicates the main outcome of
   the statement. *)
val close_stmt :
  ?merge:cont KMap.t -> stmt_env -> Typing_cont_key.t -> blank_env * cont KMap.t

(* get_gpc returns the current global pc policy *)
val get_gpc : renv -> (yes, 'a) env -> PSet.t

(* get_lpc returns the current local pc policy *)
val get_lpc : (yes, 'a) env -> PSet.t

(* with_pc runs the sub-analysis provided as function argument with
   updated control-flow dependencies.
   This API ensures that pc dependencies evolve according to
   a stack discipline in envs *)
val with_pc :
  stmt_env -> PSet.t -> (stmt_env -> blank_env * 'a) -> blank_env * 'a

(* Same as with_pc but adds to the existing control-flow dependencies
   instead of replacing them *)
val with_pc_deps :
  stmt_env -> PSet.t -> (stmt_env -> blank_env * 'a) -> blank_env * 'a

val get_locals : (yes, 'a) env -> ptype LMap.t

val get_next : (yes, 'a) env -> cont

(* get_deps return the set of static callables which were accumulated
   in the environment *)
val get_deps : ('a, 'b) env -> SSet.t

(* add_dep adds a dependency in the dependency accumulator of th
   environment *)
val add_dep : ('a, 'b) env -> string -> ('a, 'b) env

(* acc is used to accumulate constraints in the environment *)
val acc : ('a, 'b) env -> (prop list -> prop list) -> ('a, 'b) env

val get_constraints : ('a, 'b) env -> prop list

(* throw registers that the expression currently being checked
   can throw an exception; the exception may or may not be thrown
   depending on the value of some data subject to the policies
   passed as second argument *)
val throw : expr_env -> PSet.t -> expr_env

(* analyze_lambda_body is used to check the body of a lambda; it
   is necessary to transition to a stmt_env in an expr_env
   (the environment in which the lambda occured) without having
   to close the expr_env *)
val analyze_lambda_body : expr_env -> (blank_env -> blank_env) -> expr_env

val get_local_type : (yes, 'a) env -> Local_id.t -> ptype option

val set_local_type : (yes, 'a) env -> Local_id.t -> ptype -> (yes, 'a) env

(* Update or remove a local from the env *)
val set_local_type_opt :
  (yes, 'a) env -> Local_id.t -> ptype option -> (yes, 'a) env

(* - Outcomes - *)

val empty_cont : cont

(* merge_out merges outcomes; if a continuation is assigned
   a local env only in one of the arguments it will be kept
   as is in the result. This is because lack of a continuation
   means that some code was, up to now, dead code. *)
val merge_out : cont KMap.t -> cont KMap.t -> cont KMap.t

(* strip_cont removes and returns a continuation of an outcome *)
val strip_cont : cont KMap.t -> Typing_cont_key.t -> cont KMap.t * cont option

(* merge_in_next will erase the outcome designated by its second
   argument and merge it in the fallthrough (Next) outcome; this
   function also works when the argument continuation is Next
   itself (in this case, it does not change the outcome map) *)
val merge_in_next : cont KMap.t -> Typing_cont_key.t -> cont KMap.t

(* Same as merge_in_next except that it merges the fallthrough
   outcome in the one designated by the second argument *)
val merge_next_in : cont KMap.t -> Typing_cont_key.t -> cont KMap.t
