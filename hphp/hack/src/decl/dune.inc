(library
  (name decl)
  (wrapped false)
  (libraries
    ast_provider
    full_fidelity
    heap_global_storage
    naming
    naming_attributes
    partial_provider
    procs_procs
    typing_defs
  )
  (preprocess (pps ppx_deriving.std)))
