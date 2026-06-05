open Hh_prelude

(** [go_fast] uses the dependency graph to find files that *might*
    depend on definitions in [file].  This is over-approximate: it
    includes any file whose dependency-table entry touches a name
    defined here, even for edges (like type annotations) that the
    package checker doesn't enforce.

    Returns [(env, file_set)] — the server environment (potentially
    updated by prechecked-file bookkeeping) and a set of candidate
    file paths. *)
let go_fast _genv env file =
  let ctx = Provider_utils.ctx_from_server_env env in
  let path = Relative_path.create_detect_prefix file in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  (* Phase 1: Collect all dep hashes from AST definitions *)
  let open Typing_deps in
  let dep_set =
    List.fold ast ~init:(DepSet.make ()) ~f:(fun deps def ->
        let open Aast_defs in
        match def with
        | Class { c_name = (_, name); _ }
        | Typedef { t_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.Type name))
        | Fun { fd_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.Fun name))
        | Constant { cst_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.GConst name))
        | ClassAlias _
        | Stmt _
        | Namespace _
        | FileAttributes _
        | Module _
        | NamespaceUse _
        | SetNamespaceEnv _
        | SetModule _ ->
          deps)
  in
  (* Phase 2: Single batched dep graph lookup + single file resolution *)
  let deps_mode = Provider_context.get_deps_mode ctx in
  let all_ideps = add_typing_deps deps_mode dep_set in
  let files = Naming_provider.get_files ctx all_ideps in
  (env, files)

(* --- Helpers for [go] ------------------------------------------------- *)

(** Fully-qualified names of every top-level definition in [ast]
    (classes, typedefs, functions, constants).  These are the names
    that [references_target] checks symbol occurrences against.
    Names are already fully qualified (with leading backslash) because
    [Ast_provider.compute_ast] returns a named AST (NAST). *)
let target_names_of_ast ast =
  List.fold ast ~init:SSet.empty ~f:(fun acc def ->
      let open Aast_defs in
      match def with
      | Class { c_name = (_, name); _ }
      | Typedef { t_name = (_, name); _ }
      | Fun { fd_name = (_, name); _ }
      | Constant { cst_name = (_, name); _ } ->
        SSet.add name acc
      | ClassAlias _
      | Stmt _
      | Namespace _
      | FileAttributes _
      | Module _
      | NamespaceUse _
      | SetNamespaceEnv _
      | SetModule _ ->
        acc)

(** Does [sym] reference any name in [names]?  Only matches Class,
    Function, and GConst occurrences — these are the top-level
    definition kinds that [target_names_of_ast] collects.  Member-level
    kinds (Method, Property, ClassConst, Typeconst) are skipped because
    any member access implies a Class occurrence on the same receiver,
    so the class-level match is sufficient. *)
let references_target names sym =
  let module SO = SymbolOccurrence in
  match sym.SO.type_ with
  | SO.Class _
  | SO.Function
  | SO.GConst ->
    SSet.mem sym.SO.name names
  | _ -> false

(** Does [cand] contain at least one production-affecting reference to
    any of [target_names]?  Computes the TAST for [cand] and checks
    symbol occurrences.  [affects_prod_build] filters out occurrences
    that don't trigger package enforcement at runtime — nameof
    expressions, [::class] constants, attribute references, etc. *)
let has_prod_ref_to ctx target_names cand =
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path:cand in
  (* Use unquarantined because this is a batch operation across multiple
     files via MultiWorker, not a single-file IDE request.  Quarantined
     mode is only appropriate for single-file IDE operations. *)
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_unquarantined ~ctx ~entry
  in
  IdentifySymbolService.all_symbols
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions
  |> List.exists ~f:(fun sym ->
         sym.SymbolOccurrence.affects_prod_build
         && references_target target_names sym)

(** [go _genv env file candidate_files] answers: "If we removed the
    __PackageOverride from [file], which of [candidate_files] would
    lose access to its symbols?"

    For each candidate, computes its TAST and checks whether any
    production-affecting symbol occurrence (i.e. [affects_prod_build =
    true]) references a definition from [file].  This filters out
    nameof expressions, ::class references, attributes, and other
    non-package-enforced edges.  Then filters by package relationship:
    keeps only candidates whose package cannot access the target's
    natural (path-based) package.

    If the returned set is empty the override is redundant. *)
let go genv env file candidate_files =
  let ctx = Provider_utils.ctx_from_server_env env in
  let path = Relative_path.create_detect_prefix file in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  let target_names = target_names_of_ast ast in
  let candidate_paths =
    List.map candidate_files ~f:Relative_path.create_detect_prefix
  in
  (* For each candidate file, check if it contains at least one
     production-affecting reference to any of the target's
     definitions.  Use MultiWorker to parallelize TAST computation. *)
  let job acc candidates =
    List.fold candidates ~init:acc ~f:(fun acc cand ->
        if has_prod_ref_to ctx target_names cand then
          Relative_path.Set.add acc cand
        else
          acc)
  in
  let ref_files =
    MultiWorker.call
      genv.ServerEnv.workers
      ~job
      ~merge:Relative_path.Set.union
      ~neutral:Relative_path.Set.empty
      ~next:(MultiWorker.next genv.ServerEnv.workers candidate_paths)
  in
  (* Target uses its natural (path-based) package because the question is
     "what if the override on the target were removed?". Dependents use
     their *effective* package (override-honored) — they're not part of
     that hypothetical. *)
  let tcopt = Provider_context.get_tcopt ctx in
  let pkg_info = TypecheckerOptions.package_info tcopt in
  let support_multifile_tests =
    TypecheckerOptions.package_support_multifile_tests tcopt
  in
  let target_suffix = Relative_path.suffix path in
  let natural_pkg =
    Package_info.get_package_for_file
      ~support_multifile_tests
      pkg_info
      ~path:target_suffix
  in

  match natural_pkg with
  | None ->
    (* Target file has no natural package (not covered by PACKAGES.toml).
       We can't reason about access, so return all refs conservatively. *)
    (env, ref_files)
  | Some natural_pkg ->
    let filtered =
      Relative_path.Set.filter ref_files ~f:(fun dep_path ->
          (* Same-file access is always allowed; matches
             [Typing_packages.can_access_by_package_rules]. *)
          if Relative_path.equal dep_path path then
            false
          else
            let dep_pkg =
              match File_provider.get_contents dep_path with
              | None -> None
              | Some text ->
                let (pkg, _has_override) =
                  Package_info.get_package_with_override_for_file_no_env
                    ~support_multifile_tests
                    pkg_info
                    ~path:(Relative_path.suffix dep_path)
                    ~content:text
                in
                pkg
            in

            match dep_pkg with
            | None ->
              (* Dep has no package — conservatively assume it needs
                 the override (we can't prove it doesn't). *)
              true
            | Some dep_pkg ->
              (* Check whether dep_pkg can access natural_pkg:
                 - Equal:         same package, always has access
                 - Includes:      dep_pkg hard-includes natural_pkg
                 - Soft_includes: not enforced, would break
                 - Unrelated:     no include path, would break *)
              (match Package.relationship dep_pkg natural_pkg with
              | Package.Equal
              | Package.Includes ->
                false
              | Package.Soft_includes
              | Package.Unrelated ->
                true))
    in
    (env, filtered)
