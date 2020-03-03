(library
  (name affectedDeps)
  (modules affectedDeps)
  (libraries
    decl_defs
    typing_defs
    typing_deps)
  (preprocess (pps ppx_deriving.std)))

(library
  (name classDiff)
  (modules classDiff)
  (libraries
    decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl)
  (modules
    decl
    decl_inherit
    decl_requirements
    direct_decl_parser)
  (libraries
    affectedDeps
    classDiff
    ast_provider
    decl_class
    decl_class_elements
    decl_enum
    decl_env
    decl_fun_utils
    decl_heap
    decl_init_check
    decl_pos_utils
    decl_to_typing
    full_fidelity
    naming
    naming_heap
    shallow_class_diff
    shallow_classes_provider
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_ancestors)
  (modules decl_ancestors)
  (libraries
    decl_defs
    decl_linearize
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_class)
  (modules decl_class)
  (libraries
    decl_defs
    decl_heap
    decl_instantiate
    decl_subst
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_class_elements)
  (modules decl_class_elements)
  (libraries
    decl_defs
    decl_heap
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_compare)
  (modules decl_compare)
  (libraries
    decl_defs
    decl_heap
    decl_pos_utils
    provider_context
    typechecker_options
    typing_defs
    typing_deps)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_compare_utils)
  (modules decl_compare_utils)
  (libraries
    classDiff
    decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_defs)
  (modules decl_defs)
  (libraries
    typing_defs
    utils_core)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_enum)
  (modules decl_enum)
  (libraries
    decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_env)
  (modules decl_env)
  (libraries
    decl_defs
    decl_heap
    provider_context
    typing_defs
    typing_deps)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_export)
  (modules decl_export)
  (libraries
    decl
    decl_defs
    decl_heap
    naming
    naming_heap
    utils_core)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_fun_utils)
  (modules decl_fun_utils)
  (libraries
    decl_defs
    decl_env
    decl_hint
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_heap)
  (modules decl_heap)
  (libraries
    decl_defs
    heap_shared_mem
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_hint)
  (modules decl_hint)
  (libraries
    decl_defs
    decl_env
    partial_provider
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_inheritance)
  (modules decl_inheritance)
  (libraries
    decl_defs
    decl_enum
    decl_instantiate
    decl_linearize
    decl_to_typing
    shallow_classes_provider
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_init_check)
  (modules decl_init_check)
  (libraries
    decl_defs
    decl_env
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_instantiate)
  (modules decl_instantiate)
  (libraries
    decl_defs
    decl_subst
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_linearize)
  (modules decl_linearize)
  (libraries
    decl_defs
    decl_env
    decl_instantiate
    decl_subst
    decl_utils
    shallow_decl_defs
    shallow_classes_provider
    linearization_provider
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_pos_utils)
  (modules decl_pos_utils)
  (libraries
    decl_defs
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_service)
  (modules
    decl_service)
  (libraries
    decl
    procs_procs
    utils_core)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_redecl_service)
  (modules
    decl_redecl_service)
  (libraries
    affectedDeps
    decl
    decl_class_elements
    decl_compare
    decl_linearize
    heap_global_storage
    naming
    procs_procs
    shallow_decl_compare
    typing_deps
    utils_core)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_subst)
  (modules decl_subst)
  (libraries
    decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_to_typing)
  (modules decl_to_typing)
  (libraries
    decl_defs
    decl_instantiate
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name decl_utils)
  (modules decl_utils)
  (libraries
    decl_defs
    naming_attributes
    naming_attributes_deprecated
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_class_diff)
  (modules shallow_class_diff)
  (libraries
    classDiff
    decl_defs
    decl_pos_utils
    decl_utils
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_class_fanout)
  (modules shallow_class_fanout)
  (libraries
    affectedDeps
    classDiff
    decl_defs
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_classes_heap)
  (modules shallow_classes_heap)
  (libraries
    decl_defs
    heap_shared_mem
    naming
    naming_heap
    shallow_decl
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_decl)
  (modules shallow_decl)
  (libraries
    decl_defs
    decl_fun_utils
    decl_utils
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_decl_compare)
  (modules shallow_decl_compare)
  (libraries
    affectedDeps
    classDiff
    decl_compare_utils
    decl_defs
    shallow_class_diff
    shallow_class_fanout
    shallow_classes_provider
    shallow_decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))

(library
  (name shallow_decl_defs)
  (modules shallow_decl_defs)
  (libraries
    decl_defs
    typing_defs)
  (preprocess (pps ppx_deriving.std)))
