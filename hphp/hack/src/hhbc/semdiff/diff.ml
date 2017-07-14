(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* TODO: change this over to Core *)

module Log = Semdiff_logging

let concatstrs = String.concat ""

let substedit s1 s2 = [Log.Del s1; Log.Add s2]

(* TODO: replace list with tree-type thingy for concatenation efficiency *)
type edit_sequence = Log.tagged_string list

let mymin (a,va) (b,vb) = if a<b then (a,va) else (b,vb)
let keys_of al = List.map fst al
let difference l1 l2 = List.filter (fun k -> not (List.mem k l2)) l1
let intersect l1 l2 = List.filter (fun k -> List.mem k l2) l1

let sumsize elt_size l = List.fold_left (fun n e -> n + elt_size e) 0 l

type 'a compare = {
  comparer : 'a -> 'a -> int * (int * edit_sequence);
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
 | Subedit of edit_sequence (* TODO: something more structured? *)

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
      | (_,Subedit edits) -> readback (i-1) (j-1) (edits @ sofar)
      | (_,Insert) ->
       readback i (j-1) ([Log.Add (value_comparer.string_of a2.(j-1))] @ sofar)
      | (_,Delete) ->
       readback (i-1) j ([Log.Del (value_comparer.string_of a1.(i-1))] @ sofar) in
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
  (n, (totalsize, readback len1 len2 []))

(* In contrast to the above, a really stupid pointwise comparer for lists
   that runs in linear time and stops producing detailed diff outputs after
   a threshold has been reached.
   One could usefully replace this with something better, but still linear.
*)
let max_array_size = 1000000
let edit_cost_threshold = 20

let rec dumb_compare value_comparer l1 l2 costsofar sizesofar edit_list =
 let possibly_remove x x_xsize =
   if costsofar < edit_cost_threshold then
      let new_list = edit_list @ [Log.Del (value_comparer.string_of x)] in
      if costsofar+x_xsize >= edit_cost_threshold then
        new_list @ [Log.Def "...truncated..."]
      else new_list
   else edit_list in

  let possibly_add x x_xsize =
    if costsofar < edit_cost_threshold then
       let new_list = edit_list @  [Log.Add (value_comparer.string_of x)] in
        if costsofar+x_xsize >= edit_cost_threshold then
          new_list @ [Log.Def "...truncated..."]
        else new_list
    else edit_list in

  let possibly_edit edits size =
    if costsofar < edit_cost_threshold then
      let new_list = edit_list @ edits in
       if costsofar + size >= edit_cost_threshold then
          new_list @ [Log.Def "...truncated..."]
       else new_list
    else edit_list in

 match l1, l2 with
  | [], [] -> (costsofar, (sizesofar, edit_list))
  | x::xs, [] -> let x_size = value_comparer.size_of x in
                  dumb_compare value_comparer xs []
                    (costsofar + x_size)
                    (sizesofar + x_size)
                    (possibly_remove x x_size)
  | [], y::ys -> let y_size = value_comparer.size_of y in
                  dumb_compare value_comparer [] ys
                    (costsofar + y_size)
                    (sizesofar + y_size)
                    (possibly_add y y_size)
  | x::xs, y::ys ->
    let (editcost,(size,edits)) = value_comparer.comparer x y in
    if editcost = 0 then
          dumb_compare value_comparer xs ys costsofar (size+sizesofar) edit_list
    else  dumb_compare value_comparer xs ys (editcost+costsofar)
            (size+sizesofar) (possibly_edit edits editcost)

let falling_back_list_comparer value_comparer l1 l2 =
  let len1 = List.length l1 in
  let len2 = List.length l2 in
   if len1 * len2 < max_array_size then levenshtein value_comparer l1 l2
   else (Log.debug (Tty.Normal Tty.Blue) "****Falling back on dumb comparer";
         dumb_compare value_comparer l1 l2 0 0 [])

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
  comparer = (fun n1 n2 -> if n1=n2 then (0,(1,[]))
                     else (1,(1,substedit (to_string n1) (to_string n2))));
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
     (d1+d2, (s1+s2, e1 @ e2))); (* definitely want better combinations here *)
  size_of = (fun a -> (c1.size_of a) + (c2.size_of a));
  string_of = (fun a -> combinestrings (c1.string_of a) (c2.string_of a));
}

let rec joindiffs diffs = match diffs with
 | [] -> (0,(0,[]))
 | (d1,(s1,e1)) :: rest -> let (d,(s,e)) = joindiffs rest
                           in (d+d1, (s+s1, e1 @ e))

let joinmap f l = joindiffs (List.map f l)

(* the comparer that says everything is equal *)
let true_comparer vtostring = {
  comparer = (fun _v1 _v2 -> (0,(0,[])));
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
     let only1 = List.map (fun v -> Log.Del (vtostring v))
                          (difference s1 s2) in
     let only2 = List.map (fun v -> Log.Add (vtostring v))
                          (difference s2 s1) in
       (List.length only1 + List.length only2,
         (List.length s1, only1 @ only2)));
  size_of = List.length;
  string_of =
   (fun l -> "{" ^ concatstrs (List.map (fun v -> vtostring v ^ " ") l) ^ "}");
}

let option_comparer value_comparer = {
  comparer = (fun o1 o2 -> match o1, o2 with
               | None, None -> (0,(1,[]))
               | Some v1, None -> (value_comparer.size_of v1,
                                   (value_comparer.size_of v1,
                                    [Log.Del (value_comparer.string_of v1)]))
               | None, Some v2 ->(value_comparer.size_of v2,
                                   (value_comparer.size_of v2,
                                     [Log.Add (value_comparer.string_of v2)]))
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
               (s,(s, [Log.Del ((ktostring k) ^ "->" ^ (vtostring v))])))
             k1only in
  let adds =
    joinmap (fun k -> let v = List.assoc k al2 in
                      let s = vsize v in
                      (s,(s, [Log.Add ((ktostring k) ^ "->" ^ (vtostring v))])))
             k2only in
  let diffs = joinmap (fun k -> let v1 = List.assoc k al1 in
                                let v2 = List.assoc k al2 in
                                Log.debug (Tty.Normal Tty.Blue) @@
                                   Printf.sprintf "comparing key %s" (ktostring k);
                                let (d,(s,e)) = value_comparer.comparer v1 v2 in
                                let expanded_edits =
                                  if d=0 then e
                                  else [Log.Def ("for " ^ (ktostring k) ^ ":")] @ e in
                                (d,(s,expanded_edits)))
                     both in
   joindiffs [dels; adds; diffs]);
   size_of = (fun l -> sumsize (fun (_k,v) -> value_comparer.size_of v) l);
   string_of = (fun l -> "[" ^ (concatstrs (List.map (fun (k,v) -> (ktostring k)
                ^ "->" ^ (value_comparer.string_of v) ^ ";") l)) ^ "]");
}

let flag_comparer name = {
  comparer = (fun b1 b2 -> match b1,b2 with
              | false, false
              | true, true -> (0, (1,[]))
              | false, true -> (1,(1, [Log.Add name]))
              | true, false -> (1,(1, [Log.Del name]))
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

(* map of function names to functions
   now only selects the top-level ones - others are compared dynamically
*)
let top_functions_alist_of_program p =
  List.map (fun f -> (Hhbc_id.Function.to_raw_string (Hhas_function.name f), f))
           (List.filter Hhas_function.is_top @@ Hhas_program.functions p)

let methods_alist_of_class c =
 List.map (fun m -> (Hhbc_id.Method.to_raw_string (Hhas_method.name m), m))
          (Hhas_class.methods c)

let name_comparer = string_comparer
let param_name_comparer = wrap Hhas_param.name
                               (fun _p s -> s) name_comparer
let param_is_reference_comparer = wrap Hhas_param.is_reference
                               (fun p _s -> if Hhas_param.is_reference p
                                            then "&" else "")
                               bool_comparer
let param_is_variadic_comparer = wrap Hhas_param.is_variadic
                               (fun p _s -> if Hhas_param.is_variadic p
                                            then "..." else "")
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
let param_variadic_type_info_comparer =
  join (fun s1 s2 -> s1 ^ s2)
  param_is_variadic_comparer
  param_type_info_comparer
let param_name_reference_comparer =
  join (fun s1 s2 -> s1 ^ s2)
  param_is_reference_comparer
  param_name_comparer
let param_ti_name_reference_comparer =
  join (fun s1 s2 -> s1 ^ s2)
  param_variadic_type_info_comparer
  param_name_reference_comparer
(* Lifting the above to work on the first component of a pair *)
let param_ti_name_reference_comparer_lifted =
 wrap fst (fun (_param, (_instrs : Hhbc_ast.instruct list)) s -> s)
          param_ti_name_reference_comparer


(* fix this *)
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

let unmangled_name_comparer =
 wrap Hhbc_string_utils.Closures.unmangle_closure
   (fun _ s -> s) (option_comparer string_comparer)

let class_comparer =
  wrap Hhbc_id.Class.to_raw_string (fun _ s -> s) unmangled_name_comparer

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

(* apply a permutation to the trailing elements of a list
   used to reorder the properties of one closure class to
   match them up with those of a corresponding one *)
let permute_property_list perm ps =
 let offset = List.length ps - List.length perm in
 let sorted_perm = List.sort (fun (a,_) (b,_) -> compare a b) perm in
 let permuted_tail = List.map (fun (_,i) -> List.nth ps (offset+i)) sorted_perm in
  Core.List.take ps offset @ permuted_tail

let property_list_comparer perm =
  let lc = list_comparer property_comparer "\n" in
{
  comparer = (fun l1 l2 ->
  let permuted_l2 = permute_property_list perm l2 in
  (if perm = [] then ()
   else let l1names = concatstrs
             (List.map (fun p -> Hhbc_id.Prop.to_raw_string (Hhas_property.name p)) l1) in
        let l2names = concatstrs
             (List.map (fun p -> Hhbc_id.Prop.to_raw_string (Hhas_property.name p)) permuted_l2) in
        Log.debug (Tty.Normal Tty.Blue) @@ Printf.sprintf "properties %s and %s" l1names l2names);
   lc.comparer l1 permuted_l2);
  size_of = lc.size_of;
  string_of = lc.string_of;
}

let class_properties_comparer perm =
 wrap Hhas_class.properties (fun _ s -> s) (property_list_comparer perm)

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

(* checking declvars as a list, not a set, as order does matter
   we also take a permutation, though this should always be the
   identity except in the case of a closure class's single
   method
 *)
let permute_decl_list perm ds =
 if perm = [] then ds
 else let sorted_perm = List.sort (fun (a,_) (b,_) -> compare a b) perm in
      let sorted_section = List.map (fun (_,i) -> List.nth ds (1+i)) sorted_perm in
      (List.hd ds) :: (sorted_section @ Core.List.drop ds (List.length perm + 1))

let decl_list_comparer perm =
  let lc = list_comparer string_comparer "," in
  {
    comparer = (fun l1 l2 ->
    let permuted_l2 = permute_decl_list perm l2 in
    (if perm = [] then ()
     else let l1names = concatstrs l1 in
          let l2names = concatstrs permuted_l2 in
          Log.debug (Tty.Normal Tty.Blue) @@ Printf.sprintf "declvars %s and %s" l1names l2names);
     lc.comparer l1 permuted_l2);
    size_of = lc.size_of;
    string_of = lc.string_of;
   }

let body_decl_vars_comparer perm =
  wrap Hhas_body.decl_vars (fun _f s -> s) (decl_list_comparer perm)

let body_num_iters_comparer =
wrap Hhas_body.num_iters (fun _ s -> "numiters = " ^ s) int_comparer

let body_num_cls_ref_slots_comparer =
wrap Hhas_body.num_cls_ref_slots (fun _ s -> "numclsrefslots = " ^ s)
     int_comparer

let body_iters_cls_ref_slots_comparer =
join (fun s1 s2 -> s1 ^ "\n" ^ s2)
 body_num_iters_comparer
 body_num_cls_ref_slots_comparer

let body_iters_cls_ref_slots_decl_vars_comparer perm =
 join (fun s1 s2 -> s1 ^ "\n" ^ s2)
  body_iters_cls_ref_slots_comparer
  (body_decl_vars_comparer perm)

(* string_of_instruction already appends a newline, so remove it *)
let droplast s = String.sub s 0 (String.length s - 1)
let my_string_of_instruction i = droplast (Hhbc_hhas.string_of_instruction i)
let instruct_comparer = primitive_comparer my_string_of_instruction

let instruct_list_comparer = list_comparer instruct_comparer "\n"


(* Try the semantic differ; if it fails, drop back to syntactic one *)
let instruct_list_comparer_with_semdiff = {
  comparer = (fun l1 l2 ->
                  match Rhl.equiv l1 l2 [] with
                | None -> (Log.debug (Tty.Normal Tty.White) "Semdiff succeeded";
                           (0, (List.length l1, [])))
                | Some (pc,pc',asn,assumed,todo) ->
                  (Log.debug (Tty.Normal Tty.White) "Semdiff failed";
                  Log.debug (Tty.Normal Tty.White) @@ Printf.sprintf
                  "pc=%s, pc'=%s, i=%s i'=%s asn=%s\nAssumed=\n%s\nTodo=%s"
                  (Rhl.string_of_pc pc) (Rhl.string_of_pc pc')
                  (my_string_of_instruction
                    (List.nth l1 (Rhl.ip_of_pc pc)))
                  (my_string_of_instruction
                    (List.nth l2 (Rhl.ip_of_pc pc' )))
                  (Rhl.asntostring asn) (Rhl.labasnsmaptostring assumed)
                  (Rhl.labasnlisttostring todo);
                  instruct_list_comparer.comparer l1 l2)
             );
  size_of = instruct_list_comparer.size_of;
  string_of = instruct_list_comparer.string_of;
}

let option_get o = match o with | Some v -> v | None -> failwith "option"
let option_is_some o = match o with Some _ -> true | None -> false

(* compare two bodies' instructions, with extra entry points added for
   default parameter values, dropping back to syntactic diff on failure
*)
let body_instrs_comparer = {
  comparer = (fun b b' ->
      let todo = match Core.List.zip (Hhas_body.params b) (Hhas_body.params b') with
        | None -> [] (* different lengths so just look at initial entry point *)
        | Some param_pairs ->
          let params_with_defaults = List.filter
           (fun (p,p') -> option_is_some (Hhas_param.default_value p) &&
                          option_is_some (Hhas_param.default_value p')) param_pairs in
          List.map  (fun (p,p') ->
           (fst (option_get (Hhas_param.default_value p)),
            fst (option_get (Hhas_param.default_value p')))) params_with_defaults
      in
      let inss = Instruction_sequence.instr_seq_to_list (Hhas_body.instrs b) in
      let inss' = Instruction_sequence.instr_seq_to_list (Hhas_body.instrs b') in
        match Rhl.equiv inss inss' todo with
           | None -> (Log.debug (Tty.Normal Tty.White) "Semdiff succeeded";
                      (0, (List.length inss, [])) )
           | Some (pc,pc',asn,assumed,todo) ->
             (Log.debug (Tty.Normal Tty.White) "Semdiff failed";
             Log.debug (Tty.Normal Tty.White) @@ Printf.sprintf
             "pc=%s, pc'=%s, i=%s i'=%s asn=%s\nAssumed=\n%s\nTodo=%s"
             (Rhl.string_of_pc pc) (Rhl.string_of_pc pc')
             (my_string_of_instruction
               (List.nth inss (Rhl.ip_of_pc pc)))
             (my_string_of_instruction
               (List.nth inss' (Rhl.ip_of_pc pc' )))
             (Rhl.asntostring asn) (Rhl.labasnsmaptostring assumed)
             (Rhl.labasnlisttostring todo);
             instruct_list_comparer.comparer inss inss'));
    size_of = (fun b -> instruct_list_comparer.size_of
      (Instruction_sequence.instr_seq_to_list (Hhas_body.instrs b)));
    string_of = (fun b -> instruct_list_comparer.string_of
                 (Instruction_sequence.instr_seq_to_list (Hhas_body.instrs b)));
}

let body_comparer perm =
 join (fun s1 s2 -> s1 ^ "\n" ^ s2)
      (body_iters_cls_ref_slots_decl_vars_comparer perm)
      body_instrs_comparer

let function_body_comparer =
  wrap Hhas_function.body (fun _ s -> s) (body_comparer [])

let method_body_comparer perm =
  wrap Hhas_method.body (fun _ s -> s) (body_comparer perm)

let function_header_body_comparer =
 join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}\n") function_header_comparer
                                             function_body_comparer

let method_header_body_comparer perm =
join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}\n") method_header_comparer
                                            (method_body_comparer perm)

let program_main_comparer =
 wrap Hhas_program.main (fun _p s -> s) (body_comparer [])

let functions_alist_comparer =
 alist_comparer function_header_body_comparer (fun fname -> fname)

let methods_alist_comparer perm =
 alist_comparer (method_header_body_comparer perm) (fun mname -> mname)

let class_methods_comparer perm = wrap methods_alist_of_class
                           (fun _c s -> s) (methods_alist_comparer perm)

let class_properties_methods_comparer perm =
 join (fun s1 s2 -> s1 ^ s2)
      (class_properties_comparer perm)
      (class_methods_comparer perm)

let class_header_properties_methods_comparer perm =
 join (fun s1 s2 -> s1 ^ "{\n" ^ s2 ^ "}")
   class_header_comparer
   (class_properties_methods_comparer perm)



(* TODO: add all the other bits to classes *)
let class_comparer perm = class_header_properties_methods_comparer perm

let program_top_functions_comparer = wrap top_functions_alist_of_program
                           (fun _p s -> s) functions_alist_comparer

let program_main_top_functions_comparer =
 join (fun s1 s2 -> s1 ^ s2) program_main_comparer program_top_functions_comparer

(* Refactoring so that all comparison of classes is triggered off the dynamic
   use of DefCls etc. in main and top-level functions (and then recursively by
   methods therein), rather than by a static association list.
   This should even do the "right" thing in the case that there
   are multiple classes with the same name that are dynamically registered
   Now doing the same kind of thing for nontop-level functions *)
let classes_todosplitter s =
  if Rhl.IntIntPermSet.is_empty s then None
  else let iip = Rhl.IntIntPermSet.choose s in
           Some (iip, Rhl.IntIntPermSet.remove iip s)

let functions_todosplitter s =
  if Rhl.IntIntSet.is_empty s then None
  else let ii = Rhl.IntIntSet.choose s in
           Some (ii, Rhl.IntIntSet.remove ii s)

let compare_classes_functions_of_programs p p' =
 Rhl.classes_to_check := Rhl.IntIntPermSet.empty;
 Rhl.classes_checked := Rhl.IntIntSet.empty;
 Rhl.functions_to_check := Rhl.IntIntSet.empty;
 Rhl.functions_checked := Rhl.IntIntSet.empty;
 (* clear refs here again just to be on the safe side
    start by comparing top-level stuff
 *)
 let (dist, (size,edits)) = program_main_top_functions_comparer.comparer p p' in
 let rec loop d s e =
  let td = !Rhl.classes_to_check in
   match classes_todosplitter td with
    | None -> (match functions_todosplitter !Rhl.functions_to_check with
               | None -> (d,(s,e))
               | Some ((fid,fid'),newftodo) ->
                 Rhl.functions_to_check := newftodo;
                 if Rhl.IntIntSet.mem (fid,fid') !Rhl.functions_checked
                 then loop d s e (* already done *)
                 else
                  (* Note nasty subtraction of one here. Seems like we count .main as the
                     zeroth function, which is OK provided that main is always there and
                     always first. Since hhvm complains if main isn't first, I think this
                     is OK *)
                  let actual_function = List.nth (Hhas_program.functions p) (fid - 1) in
                  let actual_function' = List.nth (Hhas_program.functions p') (fid' -1) in
                  Rhl.functions_checked := Rhl.IntIntSet.add (fid,fid') !Rhl.functions_checked;
                  Log.debug (Tty.Normal Tty.Blue) @@
                    Printf.sprintf "dynamic function comparison %d=%s and %d=%s"
                    fid (Hhbc_id.Function.to_raw_string @@ Hhas_function.name actual_function)
                    fid' (Hhbc_id.Function.to_raw_string @@ Hhas_function.name actual_function') ;
                  if Hhas_function.is_top actual_function || Hhas_function.is_top actual_function'
                  then failwith "dynamic comparison of top-level function"
                  else ();
                  let (df, (sf,ef)) = function_header_body_comparer.comparer
                                       actual_function actual_function' in
                  loop (d+df) (s+sf) (e @ ef)
              )
    | Some ((ac,ac',perm), newtodo) ->
     (Rhl.classes_to_check := newtodo;
      if Rhl.IntIntSet.mem (ac,ac') (!Rhl.classes_checked)
      then loop d s e (* already done this pair *)
      else
       let actual_class = List.nth (Hhas_program.classes p) ac in
       let actual_class' = List.nth (Hhas_program.classes p') ac' in
       Rhl.classes_checked := Rhl.IntIntSet.add (ac,ac') (!Rhl.classes_checked);
       let (dc, (sc,ec)) = (class_comparer perm).comparer actual_class actual_class' in
       (if perm = [] then ()
        else Log.debug (Tty.Normal Tty.Blue) @@
             Printf.sprintf "did perm comparison on classes %d and %d, distance was %d" ac ac' dc);
       loop (d+dc) (s+sc) (e @ ec))
 in loop dist size edits

(* TODO: sizes and printing are not quite right here,
   as they don't take nontop functions into account *)
let program_main_functions_classes_comparer = {
  comparer = compare_classes_functions_of_programs;
  size_of = (fun p -> program_main_top_functions_comparer.size_of p
                    + sumsize (class_comparer []).size_of (Hhas_program.classes p));
  string_of = (fun p -> program_main_top_functions_comparer.string_of p  ^
               String.concat "\n"
               (List.map (class_comparer []).string_of (Hhas_program.classes p)));
}

(* top level comparison for whole programs *)
let program_comparer = program_main_functions_classes_comparer
