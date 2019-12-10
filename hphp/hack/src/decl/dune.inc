(library
  (name decl)
  (wrapped false)
  (libraries
    ast_provider
    full_fidelity
    global_naming_options
    heap_global_storage
    naming
    naming_attributes
    naming_attributes_deprecated
    partial_provider
    procs_procs
    typing_defs
  )
  (preprocess (pps ppx_deriving.std)))
