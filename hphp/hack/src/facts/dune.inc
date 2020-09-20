(library
  (name facts)
  (wrapped false)
  (modules
    facts
    facts_parser
    rust_facts_ffi)
  (libraries
    core_kernel
    hhbc
    parser
    rust_facts_ffi
    (select rust_facts_ffi.ml from
      (-> ffi/rust_facts_ffi.ml)
    )
    utils_hash)
  (preprocess (pps ppx_deriving.std)))
