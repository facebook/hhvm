(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *)

(* Put any initialization code necessary here *)
let initialize ~sienv = sienv

(* If you have a way of listing namespaces, put it here *)
let fetch_namespaces ~sienv:_ = []

(* Use the custom search service to find symbols by autocomplete context *)
let search_symbols
    ~sienv_ref:_ ~query_text:_ ~max_results:_ ~context:_ ~kind_filter:_ =
  []
