(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_members = {
  m_properties: Typing_defs.decl_ty SMap.t;
  m_static_properties: Typing_defs.decl_ty SMap.t;
  m_methods: Typing_defs.fun_elt SMap.t;
  m_static_methods: Typing_defs.fun_elt SMap.t;
  m_constructor: Typing_defs.fun_elt option;
}

type class_entries = Decl_defs.decl_class_type * class_members option

module ClassEltKey = Decl_heap.ClassEltKey

type decl_store = {
  add_class: string -> Decl_defs.decl_class_type -> unit;
  get_class: string -> Decl_defs.decl_class_type option;
  add_prop: ClassEltKey.t -> Typing_defs.decl_ty -> unit;
  get_prop: ClassEltKey.t -> Typing_defs.decl_ty option;
  add_static_prop: ClassEltKey.t -> Typing_defs.decl_ty -> unit;
  get_static_prop: ClassEltKey.t -> Typing_defs.decl_ty option;
  add_method: ClassEltKey.t -> Typing_defs.fun_elt -> unit;
  get_method: ClassEltKey.t -> Typing_defs.fun_elt option;
  add_static_method: ClassEltKey.t -> Typing_defs.fun_elt -> unit;
  get_static_method: ClassEltKey.t -> Typing_defs.fun_elt option;
  add_constructor: string -> Typing_defs.fun_elt -> unit;
  get_constructor: string -> Typing_defs.fun_elt option;
  add_fun: string -> Typing_defs.fun_elt -> unit;
  get_fun: string -> Typing_defs.fun_elt option;
  add_typedef: string -> Typing_defs.typedef_type -> unit;
  get_typedef: string -> Typing_defs.typedef_type option;
  add_recorddef: string -> Typing_defs.record_def_type -> unit;
  get_recorddef: string -> Typing_defs.record_def_type option;
  add_gconst: string -> Typing_defs.const_decl -> unit;
  get_gconst: string -> Typing_defs.const_decl option;
  push_local_changes: unit -> unit;
  pop_local_changes: unit -> unit;
}

let push_local_changes () : unit =
  Decl_heap.Funs.LocalChanges.push_stack ();
  Decl_heap.RecordDefs.LocalChanges.push_stack ();
  Decl_heap.Constructors.LocalChanges.push_stack ();
  Decl_heap.Props.LocalChanges.push_stack ();
  Decl_heap.StaticProps.LocalChanges.push_stack ();
  Decl_heap.Methods.LocalChanges.push_stack ();
  Decl_heap.StaticMethods.LocalChanges.push_stack ();
  Decl_heap.Classes.LocalChanges.push_stack ();
  Decl_heap.Typedefs.LocalChanges.push_stack ();
  Decl_heap.GConsts.LocalChanges.push_stack ();
  ()

let pop_local_changes () : unit =
  Decl_heap.Funs.LocalChanges.pop_stack ();
  Decl_heap.RecordDefs.LocalChanges.pop_stack ();
  Decl_heap.Constructors.LocalChanges.pop_stack ();
  Decl_heap.Props.LocalChanges.pop_stack ();
  Decl_heap.StaticProps.LocalChanges.pop_stack ();
  Decl_heap.Methods.LocalChanges.pop_stack ();
  Decl_heap.StaticMethods.LocalChanges.pop_stack ();
  Decl_heap.Classes.LocalChanges.pop_stack ();
  Decl_heap.Typedefs.LocalChanges.pop_stack ();
  Decl_heap.GConsts.LocalChanges.pop_stack ();
  ()

let store =
  ref
    {
      add_prop = Decl_heap.Props.add;
      get_prop = Decl_heap.Props.get;
      add_static_prop = Decl_heap.StaticProps.add;
      get_static_prop = Decl_heap.StaticProps.get;
      add_method = Decl_heap.Methods.add;
      get_method = Decl_heap.Methods.get;
      add_static_method = Decl_heap.StaticMethods.add;
      get_static_method = Decl_heap.StaticMethods.get;
      add_constructor = Decl_heap.Constructors.add;
      get_constructor = Decl_heap.Constructors.get;
      add_class = Decl_heap.Classes.add;
      get_class = Decl_heap.Classes.get;
      add_fun = Decl_heap.Funs.add;
      get_fun = Decl_heap.Funs.get;
      add_typedef = Decl_heap.Typedefs.add;
      get_typedef = Decl_heap.Typedefs.get;
      add_recorddef = Decl_heap.RecordDefs.add;
      get_recorddef = Decl_heap.RecordDefs.get;
      add_gconst = Decl_heap.GConsts.add;
      get_gconst = Decl_heap.GConsts.get;
      push_local_changes;
      pop_local_changes;
    }

let set s = store := s

let get () = !store
