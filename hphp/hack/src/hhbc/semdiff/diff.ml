(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let rec concatstrs l = match l with
 | [] -> ""
 | x ::xs -> x ^ (concatstrs xs)

let addstring s = "\027[32m+ " ^ s ^ "\n"
let deletestring s = "\027[31m- " ^ s ^ "\n"
let bluestring s = "\027[34m" ^ s
let subststring s1 s2 = deletestring s1 ^ addstring s2
(* used to have something like
  bluestring "S/" ^ deletestring s1 ^ bluestring "/" ^ addstring s2
  ^ bluestring "/\n"
  but decided to express substitution as addition/deletion instead
  *)
let defaultstring = "\027[0m"
let mymin (a,va) (b,vb) = if a<b then (a,va) else (b,vb)
let keys_of al = List.map fst al
let difference l1 l2 = List.filter (fun k -> not (List.mem k l2)) l1
let intersect l1 l2 = List.filter (fun k -> List.mem k l2) l1

let sumsize elt_size l = List.fold_left (fun n e -> n + elt_size e) 0 l

type 'a compare = {
  comparer : 'a -> 'a -> int * (int * string);
  size_of : 'a -> int;
  string_of : 'a -> string
}

(* Dynamic programming implementation of Levenshtein distance for lists
   TODO: reduce memory usage by only memoizing last few rows of the table
   (but depends what I do for larger pattern matches)
   TODO: amalgamate contiguous sequences of additions/deletions into additions/
   deletions of sequences - that looks much better in the output
*)
type action =
 | Delete
 | Insert
 | Subedit of string (* TODO: something more structured? *)

let levenshtein value_comparer l1 l2 =
  let a1 = Array.of_list l1 in
  let a2 = Array.of_list l2 in
  let len1 = Array.length a1 in
  let len2 = Array.length a2 in
  let memo = Array.make_matrix (len1+1) (len2+1) (0,Insert) in (* dummy *)
  let rec readback i j sofar =
  if i = 0 && j=0 then sofar
  else
    match memo.(i).(j) with
      | (_,Subedit edits) -> readback (i-1) (j-1) (edits ^ sofar)
      | (_,Insert) ->
       readback i (j-1) (addstring (value_comparer.string_of a2.(j-1)) ^ sofar)
      | (_,Delete) ->
       readback (i-1) j (deletestring (value_comparer.string_of a1.(i-1))
                                                                 ^ sofar ) in
  for j = 1 to len2 do
    memo.(0).(j) <- let (sizesofar,_) = memo.(0).(j-1) in
                    (sizesofar + value_comparer.size_of a2.(j-1), Insert)
  done;
  for i = 1 to len1 do
    memo.(i).(0) <- (let (sizesofar,_) = memo.(i-1).(0) in
                    (sizesofar + value_comparer.size_of a1.(i-1), Delete));
    for j = 1 to len2 do
      let (c,_) = memo.(i-1).(j-1) in
      let dosubedit =
        let (editcost,(_size,edits)) =
             value_comparer.comparer a1.(i-1) a2.(j-1) in
        (c+editcost, Subedit edits) in
      let (d,_) = memo.(i-1).(j) in
      let dodel = (d+value_comparer.size_of a1.(i-1), Delete) in
      let (ins,_) = memo.(i).(j-1) in
      let doins = (ins+value_comparer.size_of a2.(j-1), Insert) in
      memo.(i).(j) <- mymin dosubedit (mymin dodel doins)
    done;
  done;
  let (n,_) = memo.(len1).(len2) in
  let (totalsize,_) = memo.(len1).(0) in
  (n, (totalsize, readback len1 len2 ""))

(* In contrast to the above, a really stupid pointwise comparer for lists
   that runs in linear time and stops producing detailed diff outputs after
   a threshold has been reached.
   One could usefully replace this with something better, but still linear.
*)
let max_array_size = 1000000
let edit_cost_threshold = 20

let rec dumb_compare value_comparer l1 l2 costsofar sizesofar buf =
 let possibly_remove x x_xsize =
   if costsofar < edit_cost_threshold then
     (Buffer.add_string buf (deletestring (value_comparer.string_of x));
      if costsofar+x_xsize >= edit_cost_threshold then
        Buffer.add_string buf "...truncated..."
      else ())
   else () in
  let possibly_add x x_xsize =
    if costsofar < edit_cost_threshold then
       (Buffer.add_string buf (addstring (value_comparer.string_of x));
        if costsofar+x_xsize >= edit_cost_threshold then
          Buffer.add_string buf "...truncated..."
        else ())
    else () in
  let possibly_edit edits size =
    if costsofar < edit_cost_threshold then
      (Buffer.add_string buf edits;
       if costsofar + size >= edit_cost_threshold then
          Buffer.add_string buf "...truncated..."
       else ())
    else () in
 match l1, l2 with
  | [], [] -> (costsofar, (sizesofar, Buffer.contents buf))
  | x::xs, [] -> let x_size = value_comparer.size_of x in
                  possibly_remove x x_size;
                  dumb_compare value_comparer xs []
                    (costsofar + x_size)
                    (sizesofar + x_size)
                    buf
  | [], y::ys -> let y_size = value_comparer.size_of y in
                  possibly_add y y_size;
                  dumb_compare value_comparer [] ys
                    (costsofar + y_size)
                    (sizesofar + y_size)
                    buf
  | x::xs, y::ys ->
    let (editcost,(size,edits)) = value_comparer.comparer x y in
    if editcost = 0 then
          dumb_compare value_comparer xs ys costsofar (size+sizesofar) buf
    else
          (possibly_edit edits editcost;
           dumb_compare value_comparer xs ys (editcost+costsofar)
            (size+sizesofar) buf)

let falling_back_list_comparer value_comparer l1 l2 =
  let len1 = List.length l1 in
  let len2 = List.length l2 in
   if len1 * len2 < max_array_size then levenshtein value_comparer l1 l2
   else (print_endline "\n****Falling back on dumb comparer";
         dumb_compare value_comparer l1 l2 0 0 (Buffer.create len1))

(* Now the default list comparer, which does levenshtein unless the input looks
   too big for a quadratic algorithm, when it falls back to dumb_compare
*)
let list_comparer elt_comparer inbetween = {
  comparer = falling_back_list_comparer elt_comparer;
  size_of = (fun l -> sumsize elt_comparer.size_of l);
  string_of = (fun l -> "[" ^ (concatstrs
     (List.map (fun elt -> elt_comparer.string_of elt ^ inbetween) l)) ^ "]");
}

let primitive_comparer to_string = {
  comparer = (fun n1 n2 -> if n1=n2 then (0,(1,""))
                     else (1,(1,subststring (to_string n1) (to_string n2))));
  size_of = (fun _n -> 1);
  string_of = to_string;
}

(* TODO: size and string representation and comparison for typed values *)
let typed_value_to_string v =
 match Typed_value.to_string v with
  | Some s -> s
  | None -> "aTypedValue"

let typed_value_comparer = primitive_comparer typed_value_to_string

let int_comparer = primitive_comparer string_of_int
let string_comparer = primitive_comparer (fun s -> s)
let bool_comparer = primitive_comparer string_of_bool

let int_list_comparer = list_comparer int_comparer "; "

let int_list_list_comparer = list_comparer int_list_comparer "; "


(* wrap takes a function a->b,a function a->string->string, a b-comparer and
   returns an a-comparer
   TODO: this is still not quite right, as we want to be able to customize the
   wrapped edits, and that should be compatible with what we do in string_of.
   The underlying comparer *should* return a more structured
   (typed) list of edits with the string_of function being applied later to
   produce the final output. If I keep going, I guess
   I'll end up reinventing lenses, or something.
*)
let wrap f wrapstring c = {
  comparer = (fun a1 a2 -> c.comparer (f a1) (f a2));
  size_of = (fun a -> c.size_of (f a)); (* could adjust these too *)
  string_of = (fun a -> wrapstring a (c.string_of (f a)))
}

(* join combines two, assumed independent, comparers on the same type *)
let join combinestrings c1 c2 = {
  comparer = (fun a b ->
     let (d1,(s1,e1)) = c1.comparer a b in
     let (d2,(s2,e2)) = c2.comparer a b in
     (d1+d2, (s1+s2, e1 ^ e2))); (* definitely want better combinations here *)
  size_of = (fun a -> (c1.size_of a) + (c2.size_of a));
  string_of = (fun a -> combinestrings (c1.string_of a) (c2.string_of a));
}

let rec joindiffs diffs = match diffs with
 | [] -> (0,(0,""))
 | (d1,(s1,e1)) :: rest -> let (d,(s,e)) = joindiffs rest
                           in (d+d1, (s+s1, e1^e))

let joinmap f l = joindiffs (List.map f l)

(* the comparer that says everything is equal *)
let true_comparer vtostring = {
  comparer = (fun _v1 _v2 -> (0,(0,"")));
  size_of = (fun _v -> 0);
  string_of = vtostring
}

(* comparer for sets represented as lists
   this only works properly for primitive types
   'cos it uses literal equality
*)
let primitive_set_comparer vtostring = {
  comparer =
   (fun s1 s2 ->
     let only1 = List.map (fun v -> deletestring (vtostring v) ^ " ")
                          (difference s1 s2) in
     let only2 = List.map (fun v -> addstring (vtostring v) ^ " ")
                          (difference s2 s1) in
       (List.length only1 + List.length only2,
         (List.length s1, concatstrs (only1 @ only2))));
  size_of = List.length;
  string_of =
   (fun l -> "{" ^ concatstrs (List.map (fun v -> vtostring v ^ " ") l) ^ "}");
}

let option_comparer value_comparer = {
  comparer = (fun o1 o2 -> match o1, o2 with
               | None, None -> (0,(1,""))
               | Some v1, None -> (value_comparer.size_of v1,
                                   (value_comparer.size_of v1,
                                    deletestring (value_comparer.string_of v1)))
               | None, Some v2 ->(value_comparer.size_of v2,
                                   (value_comparer.size_of v2,
                                     addstring (value_comparer.string_of v2)))
               | Some v1, Some v2 -> value_comparer.comparer v1 v2);
  size_of = (fun o -> match o with | None -> 1
                                   | Some v -> 1+(value_comparer.size_of v));
  string_of = (fun o -> match o with | None -> ""
                                     | Some v -> value_comparer.string_of v);
}


(* compare two maps (association lists). We use literal equality on the keys,
   match those up first and then use the value_comparer on the values
*)
let alist_comparer value_comparer ktostring = {
   comparer = (fun al1 al2 ->
  let vtostring = value_comparer.string_of in
  let vsize = value_comparer.size_of in
  let k1 = keys_of al1 in
  let k2 = keys_of al2 in
  let both = intersect k1 k2 in
  let k1only = difference k1 k2 in
  let k2only = difference k2 k1 in
  let dels =
    joinmap (fun k ->
               let v = List.assoc k al1 in
               let s = vsize v in
               (s,(s, deletestring (ktostring k) ^ "->" ^ (vtostring v))))
             k1only in
  let adds =
    joinmap (fun k -> let v = List.assoc k al2 in
                      let s = vsize v in
                      (s,(s,addstring (ktostring k) ^ "->" ^ (vtostring v))))
             k2only in
  let diffs = joinmap (fun k -> let v1 = List.assoc k al1 in
                                let v2 = List.assoc k al2 in
                                value_comparer.comparer v1 v2)
                     both in
   joindiffs [dels; adds; diffs]);
   size_of = (fun l -> sumsize (fun (_k,v) -> value_comparer.size_of v) l);
   string_of = (fun l -> "[" ^ (concatstrs (List.map (fun (k,v) -> (ktostring k)
                ^ "->" ^ (value_comparer.string_of v) ^ ";") l)) ^ "]");
}

let flag_comparer name = {
  comparer = (fun b1 b2 -> match b1,b2 with
              | false, false
              | true, true -> (0, (1,""))
              | false, true -> (1,(1, addstring name))
              | true, false -> (1,(1, deletestring name))
              );
  size_of = (fun _b -> 1);
  string_of = (fun b -> if b then name else "");
}

let function_is_async_comparer = wrap Hhas_function.is_async
                                      (fun _f s -> s) (flag_comparer "isAsync")
let function_is_generator_comparer = wrap Hhas_function.is_generator
                                  (fun _f s -> s) (flag_comparer "isGenerator")
let function_is_pair_generator_comparer = wrap Hhas_function.is_pair_generator
                              (fun _f s -> s) (flag_comparer "isPairGenerator")

let method_is_protected_comparer = wrap Hhas_method.is_protected
                              (fun _f s -> s) (flag_comparer "isProtected")
let method_is_public_comparer = wrap Hhas_method.is_public
                                  (fun _f s -> s) (flag_comparer "isPublic")
let method_is_private_comparer = wrap Hhas_method.is_private
                                  (fun _f s -> s) (flag_comparer "isPrivate")
let method_is_static_comparer = wrap Hhas_method.is_static
                                    (fun _f s -> s) (flag_comparer "isStatic")
let method_is_final_comparer = wrap Hhas_method.is_final
                                  (fun _f s -> s) (flag_comparer "isFinal")
let method_is_async_comparer = wrap Hhas_method.is_async
                                    (fun _f s -> s) (flag_comparer "isAsync")
let method_is_generator_comparer = wrap Hhas_method.is_generator
                                  (fun _f s -> s) (flag_comparer "isGenerator")
let method_is_pair_generator_comparer = wrap Hhas_method.is_pair_generator
                              (fun _f s -> s) (flag_comparer "isPairGenerator")
let method_is_closure_body_comparer = wrap Hhas_method.is_closure_body
                              (fun _f s -> s) (flag_comparer "isClosureBody")

(* Could have used fold earlier here *)
let method_flags_comparer =
List.fold_left (join (fun s1 s2 -> s1 ^ s2)) method_is_protected_comparer
[method_is_public_comparer; method_is_private_comparer;
 method_is_static_comparer; method_is_final_comparer; method_is_async_comparer;
 method_is_generator_comparer; method_is_pair_generator_comparer;
 method_is_closure_body_comparer]

let function_flags_comparer =
 join (fun s1 s2 -> s1 ^ " " ^ s2)
      function_is_async_comparer
      (join (fun s1 s2 -> s1 ^ " " ^ s2)
            function_is_generator_comparer
            function_is_pair_generator_comparer)

let params_to_string ps = "(" ^ (concatstrs (List.map Hhas_param.name ps)) ^ ")"

(* map of function names to functions *)
let functions_alist_of_program p =
  List.map (fun f -> (Hhbc_id.Function.to_raw_string (Hhas_function.name f), f))
           (Hhas_program.functions p)

let methods_alist_of_class c =
 List.map (fun m -> (Hhbc_id.Method.to_raw_string (Hhas_method.name m), m))
          (Hhas_class.methods c)

let classes_alist_of_program p =
  List.map (fun c -> (Hhbc_id.Class.to_raw_string (Hhas_class.name c),c))
          (Hhas_program.classes p)

let name_comparer = string_comparer
let param_name_comparer = wrap Hhas_param.name
                               (fun _p s -> s) name_comparer
let param_reference_comparer = wrap Hhas_param.is_reference
                               (fun p _s -> if Hhas_param.is_reference p
                                            then "&" else "")
                               bool_comparer
let tc_flags_comparer = wrap Hhas_type_constraint.flags (fun _c s -> s)
                             (primitive_set_comparer
                               Hhas_type_constraint.string_of_flag)
let tc_name_comparer = wrap Hhas_type_constraint.name (fun _c s -> s)
                            (option_comparer string_comparer)
let type_constraint_comparer = join (fun s1 s2 -> s1 ^ " " ^ s2)
                                    tc_name_comparer tc_flags_comparer
let type_info_user_type_comparer = wrap Hhas_type_info.user_type
                                        (fun _ti s -> s)
                                        (option_comparer string_comparer)
let type_info_type_constraint_comparer = wrap Hhas_type_info.type_constraint
                                              (fun _ti s -> s)
                                              type_constraint_comparer
let type_info_comparer = join (fun s1 s2 -> "<" ^ s1 ^ " " ^s2 ^ ">")
                              type_info_user_type_comparer
                              type_info_type_constraint_comparer
let param_type_info_comparer = wrap Hhas_param.type_info
                                    (fun _p s -> s)
                                    (option_comparer type_info_comparer)
let param_name_reference_comparer = join (fun s1 s2 -> s1 ^ s2)
                                         param_reference_comparer
                                         param_name_comparer
let param_ti_name_reference_comparer = join (fun s1 s2 -> s1 ^ s2)
                                            param_type_info_comparer
                                            param_name_reference_comparer
let params_comparer = list_comparer param_ti_name_reference_comparer ", "

let function_params_comparer =
 wrap Hhas_function.params
      (fun f s ->
        Hhbc_id.Function.to_raw_string (Hhas_function.name f)
        ^ "(" ^ s ^ ")") params_comparer

let method_params_comparer =
wrap Hhas_method.params
     (fun m s -> Hhbc_id.Method.to_raw_string (Hhas_method.name m)
       ^ "(" ^ s ^ ")") params_comparer

let function_params_flags_comparer =
 join (fun s1 s2 -> s1 ^ " " ^ s2)
      function_params_comparer
      function_flags_comparer

let method_params_flags_comparer =
  join (fun s1 s2 -> s1 ^ " " ^ s2)
   method_params_comparer
   method_flags_comparer

let function_return_type_comparer =
 wrap Hhas_function.return_type (fun _f s -> s)
                                (option_comparer type_info_comparer)

let method_return_type_comparer =
  wrap Hhas_method.return_type (fun _f s -> s)
                               (option_comparer type_info_comparer)

let attribute_comparer =
  join (fun s1 s2 -> s1 ^ "(" ^ s2 ^ ")")
       (wrap Hhas_attribute.name (fun _a s -> s) string_comparer)
       (wrap Hhas_attribute.arguments (fun _l s -> s)
                                      (list_comparer typed_value_comparer " "))

let function_attributes_comparer =
 wrap Hhas_function.attributes (fun _ s -> s)
     (list_comparer attribute_comparer " ")

let method_attributes_comparer =
  wrap Hhas_method.attributes (fun _ s -> s)
    (list_comparer attribute_comparer " ")

let class_attributes_comparer =
  wrap Hhas_class.attributes (fun _ s -> s)
    (list_comparer attribute_comparer " ")

let class_comparer =
  wrap Hhbc_id.Class.to_raw_string (fun _ s -> s) string_comparer

let class_base_comparer =
 wrap Hhas_class.base (fun _ s -> "extends " ^ s)
     (option_comparer class_comparer)

let class_implements_comparer =
 wrap Hhas_class.implements (fun _ s -> "implements (" ^ s ^ ")")
    (list_comparer class_comparer " ")
let class_name_comparer =
 wrap Hhas_class.name (fun _ s -> s) class_comparer

let class_name_base_implements_comparer =
 join (fun s1 s2 -> s1 ^ s2)
   class_name_comparer
   (join (fun s1 s2 -> s1^ s2)
    class_base_comparer
    class_implements_comparer)

let class_is_final_comparer =
 wrap Hhas_class.is_final (fun _f s -> s) (flag_comparer "final")
let class_is_abstract_comparer =
  wrap Hhas_class.is_abstract (fun _f s -> s) (flag_comparer "abstract")
let class_is_interface_comparer =
  wrap Hhas_class.is_interface (fun _f s -> s) (flag_comparer "interface")
let class_is_trait_comparer =
  wrap Hhas_class.is_trait (fun _f s -> s) (flag_comparer "trait")
let class_is_xhp_comparer =
  wrap Hhas_class.is_xhp (fun _f s -> s) (flag_comparer "xhp")
let class_uses_comparer =
 wrap Hhas_class.class_uses (fun _ s -> "uses " ^ s)
  (list_comparer string_comparer " ")
let class_enum_type_comparer =
  wrap Hhas_class.enum_type (fun _ s -> "enumtype " ^ s)
  (option_comparer type_info_comparer)

(* TODO: sensible formatting, split uses and enumtype off from flags *)
let class_flags_comparer =
 List.fold_left (fun c1 c2 -> join (fun s1 s2 -> s1 ^ " " ^ s2) c1 c2)
  class_is_final_comparer
  [class_is_abstract_comparer; class_is_interface_comparer;
   class_is_trait_comparer; class_is_xhp_comparer]

let class_attributes_flags_comparer =
 join (fun s1 s2 -> "[" ^ s1 ^ " " ^ s2 ^ "]")
  class_attributes_comparer
  class_flags_comparer

let class_header_comparer =
 join (fun s1 s2 -> s1 ^ s2)
  class_attributes_flags_comparer
  class_name_base_implements_comparer

(* TODO: actually plumb in the following:
    class_uses_comparer
   class_enum_type_comparer
*)

let property_is_private_comparer =
  wrap Hhas_property.is_private (fun _f s -> s) (flag_comparer "private")
let property_is_protected_comparer =
  wrap Hhas_property.is_protected (fun _f s -> s) (flag_comparer "protected")
let property_is_public_comparer =
  wrap Hhas_property.is_public (fun _f s -> s) (flag_comparer "public")
let property_is_static_comparer =
  wrap Hhas_property.is_static (fun _f s -> s) (flag_comparer "static")
let property_is_deep_init_comparer =
  wrap Hhas_property.is_deep_init (fun _f s -> s) (flag_comparer "deep_init")
let prop_comparer =
  wrap Hhbc_id.Prop.to_raw_string (fun _ s -> s) string_comparer

let property_name_comparer =
  wrap Hhas_property.name (fun _ s -> s) prop_comparer
let property_initial_value_comparer =
 wrap Hhas_property.initial_value (fun _ s -> s)
     (option_comparer typed_value_comparer)

(* TODO: format these much more sensibly *)
let property_comparer =
 List.fold_left (fun c1 c2 -> join (fun s1 s2 -> s1 ^ s2) c1 c2)
  property_is_private_comparer
  [property_is_protected_comparer; property_is_public_comparer;
  property_is_static_comparer; property_is_deep_init_comparer;
  property_name_comparer; property_initial_value_comparer]

let class_properties_comparer =
 wrap Hhas_class.properties (fun _ s -> s)
  (list_comparer property_comparer "\n")



let function_attributes_return_type_comparer =
 join (fun s1 s2 -> s1 ^ s2)
   function_attributes_comparer
   function_return_type_comparer

let method_attributes_return_type_comparer =
    join (fun s1 s2 -> s1 ^ s2)
      method_attributes_comparer
      method_return_type_comparer

let function_header_comparer =
 join (fun s1 s2 -> s1 ^ s2) function_attributes_return_type_comparer
                             function_params_flags_comparer

let method_header_comparer =
    join (fun s1 s2 -> s1 ^ s2) method_attributes_return_type_comparer
                                method_params_flags_comparer

(* TODO: check that order doesn't matter in declvars
  Andrew says $0closure variable in closure
  classes is special, and I'm a bit worried about aliasing
  between low-numbered unnamed and named - surely that means
  order does matter? *)
let body_decl_vars_comparer =
  wrap Hhas_body.decl_vars (fun _f s -> s)
                           (primitive_set_comparer (fun s -> s))

let body_num_iters_comparer =
wrap Hhas_body.num_iters (fun _ s -> "numiters = " ^ s) int_comparer

let body_num_cls_ref_slots_comparer =
wrap Hhas_body.num_cls_ref_slots (fun _ s -> "numclsrefslots = " ^ s)
     int_comparer

let body_iters_cls_ref_slots_comparer =
join (fun s1 s2 -> s1 ^ "\n" ^ s2)
 body_num_iters_comparer
 body_num_cls_ref_slots_comparer

let body_iters_cls_ref_slots_decl_vars_comparer =
 join (fun s1 s2 -> s1 ^ "\n" ^ s2)
  body_iters_cls_ref_slots_comparer
  body_decl_vars_comparer

(* string_of_instruction already appends a newline, so remove it *)
let droplast s = String.sub s 0 (String.length s - 1)
let instruct_comparer = primitive_comparer
                        (fun i -> droplast (Hhbc_hhas.string_of_instruction i))

(* Hack to deal with adata needing to be looked up on the fly when printing
   See get_array_identifier in hhbc_hhas to understand the issue
*)
let data_decls_ref1 = ref ([] : (int*Typed_value.t) list)
let data_decls_ref2 = ref ([] : (int*Typed_value.t) list)
exception Array_id
let rec get_id_from_data_decl dd (tv : Typed_value.t) =
 match dd with
  | [] -> raise Array_id
  | (n,tv') :: rest ->
    if tv = tv'
    then (n:int)
    else get_id_from_data_decl rest tv

let my_string_of_instruction dd i =
 let f s tv = s ^ " A_" ^ (string_of_int (get_id_from_data_decl dd tv)) in
 match i with
  | Hhbc_ast.ILitConst (Hhbc_ast.Array tv) -> f "Array" tv
  | Hhbc_ast.ILitConst (Hhbc_ast.Dict tv) -> f "Dict" tv
  | Hhbc_ast.ILitConst (Hhbc_ast.Vec tv) -> f "Vec" tv
  | Hhbc_ast.ILitConst (Hhbc_ast.Keyset tv) -> f "Keyset" tv
  | _ -> droplast (Hhbc_hhas.string_of_instruction i)

(* this one's for use in larger contexts, where I can't tell which
   program the instruction came from, so don't know where to look up
   the data dictionary. Could refactor the whole diff library, but
   expect a future diff to put the data dict in the program where it belongs
   Note - even when that happens, we'll still need to thread the dictionary
   through to the instruction comparison, to see if the data is the same, unless
   we compare the dictionaries themselves as components of programs, which we
   probably don't want to do, as we don't want to insist that they're identical.
*)
let stupid_string_of_instruction i =
match i with
 | Hhbc_ast.ILitConst (Hhbc_ast.Array _tv) -> "Array A_?"
 | Hhbc_ast.ILitConst (Hhbc_ast.Dict _tv) -> "Dict A_?"
 | Hhbc_ast.ILitConst (Hhbc_ast.Vec _tv) -> "Vec A_?"
 | Hhbc_ast.ILitConst (Hhbc_ast.Keyset _tv) -> "Keyset A_?"
 | _ -> droplast (Hhbc_hhas.string_of_instruction i)

let my_instruct_comparer = {
  comparer = (fun i1 i2 ->
     if i1=i2 then (0,(1,""))
     else (1,(1,subststring (my_string_of_instruction !data_decls_ref1 i1)
                            (my_string_of_instruction !data_decls_ref2 i2))));
  size_of = (fun _n -> 1);
  string_of = stupid_string_of_instruction;
}

let instruct_list_comparer = list_comparer my_instruct_comparer "\n"

let propstostring props = concatstrs (List.map (fun (v,v') -> "(" ^
(Hhbc_hhas.string_of_local_id v) ^ "," ^
       (Hhbc_hhas.string_of_local_id v') ^ ")") props)

let string_of_pc (hs,ip) = concatstrs (List.map Hhbc_hhas.string_of_label hs)
                           ^ ";" ^ string_of_int ip
let asntostring ((l1,l2),props) = "[" ^ (string_of_pc l1) ^ "," ^
  (string_of_pc l2) ^ "->" ^ (propstostring props) ^ "]\n"
let asnstostring asns = concatstrs (List.map asntostring asns)
(* Try the semantic differ; if it fails, drop back to syntactic one *)
let instruct_list_comparer_with_semdiff = {
  comparer = (fun l1 l2 ->
               match Rhl.equiv l1 l2 with
                | None -> 0, (List.length l1, "")
                | Some (_, _, _, _, _) ->
                  (instruct_list_comparer.comparer l1 l2)
             );
  size_of = instruct_list_comparer.size_of;
  string_of = instruct_list_comparer.string_of;
}

let body_comparer =
 join (fun s1 s2 -> s1 ^ "\n" ^ s2)
      body_iters_cls_ref_slots_decl_vars_comparer
      (wrap
        (fun b -> Instruction_sequence.instr_seq_to_list (Hhas_body.instrs b))
        (fun _f s -> s)
            instruct_list_comparer_with_semdiff)

let function_body_comparer =
  wrap Hhas_function.body (fun _ s -> s) body_comparer

let method_body_comparer =
  wrap Hhas_method.body (fun _ s -> s) body_comparer

let function_header_body_comparer =
 join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}\n") function_header_comparer
                                             function_body_comparer

let method_header_body_comparer =
join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}\n") method_header_comparer
                                            method_body_comparer

let program_main_comparer =
 wrap Hhas_program.main (fun _p s -> s) body_comparer

let functions_alist_comparer =
 alist_comparer function_header_body_comparer (fun fname -> fname)

let methods_alist_comparer =
 alist_comparer method_header_body_comparer (fun mname -> mname)

let class_methods_comparer = wrap methods_alist_of_class
                           (fun _c s -> s) methods_alist_comparer

let class_properties_methods_comparer =
 join (fun s1 s2 -> s1 ^ s2)
      class_properties_comparer
      class_methods_comparer

let class_header_properties_methods_comparer =
 join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}")
   class_header_comparer
   class_properties_methods_comparer

(* TODO: add all the other bits to classes *)
let class_comparer = class_header_properties_methods_comparer

let classes_alist_comparer =
 alist_comparer class_comparer (fun cname -> cname)

let program_classes_comparer = wrap classes_alist_of_program
                       (fun _p s -> s) classes_alist_comparer

let program_functions_comparer = wrap functions_alist_of_program
                            (fun _p s -> s) functions_alist_comparer

let program_main_functions_comparer =
 join (fun s1 s2 -> s1 ^ s2) program_main_comparer program_functions_comparer

let program_main_functions_classes_comparer =
 join (fun s1 s2 -> s1 ^ s2) program_main_functions_comparer
                             program_classes_comparer

(* top level comparison for whole programs *)
let program_comparer = program_main_functions_classes_comparer
