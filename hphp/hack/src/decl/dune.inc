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
    rust_decl_ffi
    (select rust_decl_ffi.ml from
      (-> ffi/rust_decl_ffi.ml)
    )
  )
  (preprocess (pps ppx_deriving.std)))
