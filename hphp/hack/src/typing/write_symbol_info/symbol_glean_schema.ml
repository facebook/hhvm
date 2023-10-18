(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *)

open Hh_json
open Hh_prelude
module Util = Symbol_json_util
module Fact_id = Symbol_fact_id

let key v = JSON_Object [("key", v)]

let id fact_id = JSON_Object [("id", Fact_id.to_json_number fact_id)]

module Extend (M : sig
  type key

  val to_json_key : key -> Hh_json.json

  val compare_key : key -> key -> int
end) =
struct
  type t =
    | Id of Fact_id.t
    | Key of M.key
  [@@deriving ord]

  let to_json = function
    | Id f -> id f
    | Key t -> key (M.to_json_key t)
end

module Src = struct
  module File = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key str = JSON_String str
    end

    include Key
    include Extend (Key)
  end

  module FileLines = struct
    module Key = struct
      type key = {
        file: File.t;
        lengths: int list;
        endsInNewline: bool;
        hasUnicodeOrTabs: bool;
      }
      [@@deriving ord]

      let to_json_key { file; lengths; endsInNewline; hasUnicodeOrTabs } =
        JSON_Object
          [
            ("file", File.to_json file);
            ( "lengths",
              JSON_Array
                (List.map ~f:(fun n -> JSON_Number (string_of_int n)) lengths)
            );
            ("endsInNewline", JSON_Bool endsInNewline);
            ("hasUnicodeOrTabs", JSON_Bool hasUnicodeOrTabs);
          ]
    end

    include Key
    include Extend (Key)
  end

  module ByteSpan = struct
    type t = {
      start: int;
      length: int;
    }
    [@@deriving ord]

    let of_pos pos = { start = fst (Pos.info_raw pos); length = Pos.length pos }

    let to_json { start; length } =
      JSON_Object
        [
          ("start", JSON_Number (string_of_int start));
          ("length", JSON_Number (string_of_int length));
        ]
  end

  module RelByteSpan = struct
    type t = {
      offset: int;
      length: int;
    }
    [@@deriving ord]

    let to_json { offset; length } =
      JSON_Object
        [
          ("offset", JSON_Number (string_of_int offset));
          ("length", JSON_Number (string_of_int length));
        ]
  end

  module ByteSpans = struct
    type t = RelByteSpan.t list [@@deriving ord]

    let to_json t = JSON_Array (List.map ~f:RelByteSpan.to_json t)
  end
end

module Hack = struct
  module Name = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k =
        (* Remove leading slash, if present, so names such as
            Exception and \Exception are captured by the same fact *)
        let basename = Utils.strip_ns k in
        JSON_String basename
    end

    include Key
    include Extend (Key)
  end

  module StringLiteral = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k = JSON_String k
    end

    include Key
    include Extend (Key)
  end

  module Context = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k = JSON_String k
    end

    include Key
    include Extend (Key)
  end

  module Type = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k =
        let ty = Utils.strip_ns k in
        JSON_String ty
    end

    include Key
    include Extend (Key)
  end

  module NamespaceQName = struct
    type key = {
      name: Name.t;
      parent: t option;
    }
    [@@deriving ord]

    and t =
      | Id of Fact_id.t
      | Key of key
    [@@deriving ord]

    let rec of_string ns =
      match Util.split_name ns with
      | None -> { name = Name.Key ns; parent = None }
      | Some (parent_ns, namespace) ->
        { name = Name.Key namespace; parent = Some (Key (of_string parent_ns)) }

    let rec to_json = function
      | Id f -> id f
      | Key t -> key (to_json_key t)

    and to_json_key { name; parent } =
      let fields : (string * Hh_json.json) list =
        ("name", Name.to_json name)
        ::
        (match parent with
        | None -> []
        | Some parent -> [("parent", to_json parent)])
      in
      JSON_Object fields

    let to_json_nested k = key (to_json_key k)
  end

  module QName = struct
    module Key = struct
      type key = {
        name: Name.t;
        namespace_: NamespaceQName.t option;
      }
      [@@deriving ord]

      let to_json_key { name; namespace_ } =
        let fields : (string * Hh_json.json) list =
          ("name", Name.to_json name)
          ::
          (match namespace_ with
          | None -> []
          | Some ns -> [("namespace_", NamespaceQName.to_json ns)])
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)

    let of_string qname =
      match Util.split_name qname with
      | None -> { name = Name.Key qname; namespace_ = None }
      | Some (ns, name) ->
        {
          name = Name.Key name;
          namespace_ = Some NamespaceQName.(Key (of_string ns));
        }
  end

  module ModuleDeclaration = struct
    module Key = struct
      type key = { name: Name.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", Name.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module ClassDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module InterfaceDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module TraitDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module TypedefDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module GlobalConstDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module FunctionDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module EnumDeclaration = struct
    module Key = struct
      type key = { name: QName.t } [@@deriving ord]

      let to_json_key { name } = JSON_Object [("name", QName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module NamespaceDeclaration = struct
    module Key = struct
      type key = { name: NamespaceQName.t } [@@deriving ord]

      let to_json_key { name } =
        JSON_Object [("name", NamespaceQName.to_json name)]
    end

    include Key
    include Extend (Key)
  end

  module ContainerDeclaration = struct
    type t =
      | Class_ of ClassDeclaration.t
      | Enum_ of EnumDeclaration.t
      | Interface_ of InterfaceDeclaration.t
      | Trait of TraitDeclaration.t
    [@@deriving ord]

    let to_json = function
      | Class_ cd -> JSON_Object [("class_", ClassDeclaration.to_json cd)]
      | Enum_ ed -> JSON_Object [("enum_", EnumDeclaration.to_json ed)]
      | Interface_ id ->
        JSON_Object [("interface_", InterfaceDeclaration.to_json id)]
      | Trait td -> JSON_Object [("trait", TraitDeclaration.to_json td)]
  end

  module ClassConstDeclaration = struct
    module Key = struct
      type key = {
        name: Name.t;
        container: ContainerDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { name; container } =
        JSON_Object
          [
            ("name", Name.to_json name);
            ("container", ContainerDeclaration.to_json container);
          ]
    end

    include Key
    include Extend (Key)
  end

  module TypeConstDeclaration = struct
    module Key = struct
      type key = {
        name: Name.t;
        container: ContainerDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { name; container } =
        JSON_Object
          [
            ("name", Name.to_json name);
            ("container", ContainerDeclaration.to_json container);
          ]
    end

    include Key
    include Extend (Key)
  end

  module PropertyDeclaration = struct
    module Key = struct
      type key = {
        name: Name.t;
        container: ContainerDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { name; container } =
        JSON_Object
          [
            ("name", Name.to_json name);
            ("container", ContainerDeclaration.to_json container);
          ]
    end

    include Key
    include Extend (Key)
  end

  module MethodDeclaration = struct
    module Key = struct
      type key = {
        name: Name.t;
        container: ContainerDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { name; container } =
        JSON_Object
          [
            ("name", Name.to_json name);
            ("container", ContainerDeclaration.to_json container);
          ]
    end

    include Key
    include Extend (Key)
  end

  module Enumerator = struct
    module Key = struct
      type key = {
        name: Name.t;
        enumeration: EnumDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { name; enumeration } =
        JSON_Object
          [
            ("name", Name.to_json name);
            ("enumeration", EnumDeclaration.to_json enumeration);
          ]
    end

    include Key
    include Extend (Key)
  end

  module Declaration = struct
    type t =
      | ClassConst of ClassConstDeclaration.t
      | Container of ContainerDeclaration.t
      | Enumerator of Enumerator.t
      | Function_ of FunctionDeclaration.t
      | GlobalConst of GlobalConstDeclaration.t
      | Method of MethodDeclaration.t
      | ModuleDeclaration of ModuleDeclaration.t
      | Namespace_ of NamespaceDeclaration.t
      | Property_ of PropertyDeclaration.t
      | TypeConst of TypeConstDeclaration.t
      | Typedef_ of TypedefDeclaration.t
    [@@deriving ord]

    let to_json = function
      | ClassConst cc ->
        JSON_Object [("classConst", ClassConstDeclaration.to_json cc)]
      | Container c ->
        JSON_Object [("container", ContainerDeclaration.to_json c)]
      | Enumerator e -> JSON_Object [("enumerator", Enumerator.to_json e)]
      | Function_ f ->
        JSON_Object [("function_", FunctionDeclaration.to_json f)]
      | GlobalConst gc ->
        JSON_Object [("globalConst", GlobalConstDeclaration.to_json gc)]
      | Namespace_ ns ->
        JSON_Object [("namespace_", NamespaceDeclaration.to_json ns)]
      | Method m -> JSON_Object [("method", MethodDeclaration.to_json m)]
      | Property_ p ->
        JSON_Object [("property_", PropertyDeclaration.to_json p)]
      | TypeConst tc ->
        JSON_Object [("typeConst", TypeConstDeclaration.to_json tc)]
      | Typedef_ td -> JSON_Object [("typedef_", TypedefDeclaration.to_json td)]
      | ModuleDeclaration md ->
        JSON_Object [("module", ModuleDeclaration.to_json md)]
  end

  module DeclarationLocation = struct
    module Key = struct
      type key = {
        declaration: Declaration.t;
        file: Src.File.t;
        span: Src.ByteSpan.t;
      }
      [@@deriving ord]

      let to_json_key { declaration; file; span } =
        JSON_Object
          [
            ("declaration", Declaration.to_json declaration);
            ("file", Src.File.to_json file);
            ("span", Src.ByteSpan.to_json span);
          ]
    end

    include Key
    include Extend (Key)
  end

  module Visibility = struct
    type t =
      | Private
      | Protected
      | Public
      | Internal
    [@@deriving ord]

    let of_visibility = function
      | Aast.Private -> Private
      | Aast.Protected -> Protected
      | Aast.Public -> Public
      | Aast.Internal -> Internal

    let to_json t =
      let num =
        match t with
        | Private -> 0
        | Protected -> 1
        | Public -> 2
        | Internal -> 3
      in
      JSON_Number (string_of_int num)
  end

  module Variance = struct
    type t =
      | Contravariant
      | Covariant
      | Invariant
    [@@deriving ord]

    let of_ast_variance = function
      | Ast_defs.Contravariant -> Contravariant
      | Ast_defs.Covariant -> Covariant
      | Ast_defs.Invariant -> Invariant

    let to_json t =
      let num =
        match t with
        | Contravariant -> 0
        | Covariant -> 1
        | Invariant -> 2
      in
      JSON_Number (string_of_int num)
  end

  module ReifyKind = struct
    type t =
      | Erased
      | Reified
      | SoftReified
    [@@deriving ord]

    let of_ast_reifyKind = function
      | Ast_defs.Erased -> Erased
      | Ast_defs.Reified -> Reified
      | Ast_defs.SoftReified -> SoftReified

    let to_json t =
      let num =
        match t with
        | Erased -> 0
        | Reified -> 1
        | SoftReified -> 2
      in
      JSON_Number (string_of_int num)
  end

  module ConstraintKind = struct
    type t =
      | As
      | Equal
      | Super
    [@@deriving ord]

    let to_json t =
      let num =
        match t with
        | As -> 0
        | Equal -> 1
        | Super -> 2
      in
      JSON_Number (string_of_int num)

    let of_ast_constraint_kind = function
      | Ast_defs.Constraint_as -> As
      | Ast_defs.Constraint_eq -> Equal
      | Ast_defs.Constraint_super -> Super
  end

  module Constraint = struct
    type t = {
      constraintKind: ConstraintKind.t;
      type_: Type.t;
    }
    [@@deriving ord]

    let to_json { constraintKind; type_ } =
      JSON_Object
        [
          ("constraintKind", ConstraintKind.to_json constraintKind);
          ("type", Type.to_json type_);
        ]
  end

  module UserAttribute = struct
    module Key = struct
      type key = {
        name: Name.t;
        parameters: string list;
        qname: QName.t option;
      }
      [@@deriving ord]

      let to_json_key { name; parameters; qname } =
        let fields =
          [
            ("name", Name.to_json name);
            ( "parameters",
              JSON_Array (List.map parameters ~f:(fun x -> JSON_String x)) );
          ]
        in
        JSON_Object
          (match qname with
          | None -> fields
          | Some qname -> ("qname", QName.to_json qname) :: fields)
    end

    include Key
    include Extend (Key)
  end

  module TypeConstKind = struct
    type t =
      | Abstract
      | Concret
      | PartiallyAbstract
    [@@deriving ord]

    let to_json t =
      let num =
        match t with
        | Abstract -> 0
        | Concret -> 1
        | PartiallyAbstract -> 2
      in
      JSON_Number (string_of_int num)

    let of_ast_const_kind = function
      | Aast.TCAbstract _ -> Abstract
      | Aast.TCConcrete _ -> Concret
  end

  module TypeParameter = struct
    type t = {
      name: Name.t;
      variance: Variance.t;
      constraints: Constraint.t list;
      attributes: UserAttribute.t list;
      reifyKind: ReifyKind.t;
    }
    [@@deriving ord]

    let to_json { name; variance; constraints; attributes; reifyKind } =
      JSON_Object
        [
          ("name", Name.to_json name);
          ("variance", Variance.to_json variance);
          ("reifyKind", ReifyKind.to_json reifyKind);
          ( "constraints",
            JSON_Array (List.map ~f:Constraint.to_json constraints) );
          ( "attributes",
            JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
        ]
  end

  module MethodOccurrence = struct
    module Key = struct
      type key = {
        name: Name.t;
        className: Name.t option;
      }
      [@@deriving ord]

      let to_json_key { name; className } =
        JSON_Object
          (("name", Name.to_json name)
          ::
          (match className with
          | None -> []
          | Some className -> [("className", Name.to_json className)]))
    end

    include Key
    include Extend (Key)
  end

  module Occurrence = struct
    type t = { method_: MethodOccurrence.t } [@@deriving ord]

    let to_json { method_ } =
      JSON_Object [("method", MethodOccurrence.to_json method_)]
  end

  module XRefTarget = struct
    type t =
      | Occurrence of Occurrence.t
      | Declaration of Declaration.t
    [@@deriving ord]

    let to_json = function
      | Declaration d -> JSON_Object [("declaration", Declaration.to_json d)]
      | Occurrence o -> JSON_Object [("occurrence", Occurrence.to_json o)]
  end

  module XRef = struct
    type t = {
      ranges: Src.ByteSpans.t;
      target: XRefTarget.t;
    }
    [@@deriving ord]

    let to_json { target; ranges } =
      JSON_Object
        [
          ("ranges", Src.ByteSpans.to_json ranges);
          ("target", XRefTarget.to_json target);
        ]
  end

  module MethodOverrides = struct
    module Key = struct
      type key = {
        derived: MethodDeclaration.t;
        base: MethodDeclaration.t;
      }
      [@@deriving ord]

      let to_json_key { derived; base } =
        JSON_Object
          [
            ("derived", MethodDeclaration.to_json derived);
            ("base", MethodDeclaration.to_json base);
          ]
    end

    include Key
    include Extend (Key)
  end

  module FileXRefs = struct
    module Key = struct
      type key = {
        file: Src.File.t;
        xrefs: XRef.t list;
      }
      [@@deriving ord]

      let to_json_key { file; xrefs } =
        JSON_Object
          [
            ("file", Src.File.to_json file);
            ("xrefs", JSON_Array (List.map xrefs ~f:XRef.to_json));
          ]
    end

    include Key
    include Extend (Key)
  end

  module TypeInfo = struct
    module Key = struct
      type key = {
        displayType: Type.t;
        xrefs: XRef.t list;
      }
      [@@deriving ord]

      let to_json_key { displayType; xrefs } =
        JSON_Object
          [
            ("displayType", Type.to_json displayType);
            ("xrefs", JSON_Array (List.map ~f:XRef.to_json xrefs));
          ]
    end

    include Key
    include Extend (Key)
  end

  module ReadonlyKind = struct
    type t = ReadOnly [@@deriving ord]

    let to_json _ = JSON_Number "0"
  end

  module Parameter = struct
    type t = {
      name: Name.t;
      type_: Type.t option;
      isInout: bool;
      isVariadic: bool;
      defaultValue: string option;
      attributes: UserAttribute.t list;
      typeInfo: TypeInfo.t option;
      readonly: ReadonlyKind.t option;
    }
    [@@deriving ord]

    let to_json
        {
          name;
          type_;
          isInout;
          isVariadic;
          defaultValue;
          attributes;
          typeInfo;
          readonly;
        } =
      let fields =
        [
          ("name", Name.to_json name);
          ("isInout", JSON_Bool isInout);
          ("isVariadic", JSON_Bool isVariadic);
          ( "attributes",
            JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
        ]
      in
      let fields =
        match type_ with
        | None -> fields
        | Some type_ -> ("type", Type.to_json type_) :: fields
      in
      let fields =
        match defaultValue with
        | None -> fields
        | Some expr ->
          ("defaultValue", JSON_String (Util.strip_nested_quotes expr))
          :: fields
      in
      let fields =
        match typeInfo with
        | None -> fields
        | Some type_info -> ("typeInfo", TypeInfo.to_json type_info) :: fields
      in
      let fields =
        match readonly with
        | None -> fields
        | Some readonly -> ("readonly", ReadonlyKind.to_json readonly) :: fields
      in
      JSON_Object fields
  end

  module Argument = struct
    type t =
      | Lit of StringLiteral.t
      | XRef of XRefTarget.t
    [@@deriving ord]

    let to_json = function
      | Lit sl -> JSON_Object [("lit", StringLiteral.to_json sl)]
      | XRef xr -> JSON_Object [("xref", XRefTarget.to_json xr)]
  end

  module MemberCluster = struct
    module Key = struct
      type key = { members: Declaration.t list } [@@deriving ord]

      let to_json_key { members } =
        JSON_Object
          [
            ( "members",
              JSON_Array (Base.List.map ~f:Declaration.to_json members) );
          ]
    end

    include Key
    include Extend (Key)
  end

  module FileDeclarations = struct
    module Key = struct
      type key = {
        file: Src.File.t;
        declarations: Declaration.t list;
      }
      [@@deriving ord]

      let to_json_key { file; declarations } =
        JSON_Object
          [
            ("file", Src.File.to_json file);
            ( "declarations",
              JSON_Array (Base.List.map ~f:Declaration.to_json declarations) );
          ]
    end

    include Key
    include Extend (Key)
  end

  module InheritedMembers = struct
    module Key = struct
      type key = {
        container: ContainerDeclaration.t;
        inheritedMembers: MemberCluster.t list;
      }
      [@@deriving ord]

      let to_json_key { container; inheritedMembers } =
        JSON_Object
          [
            ("container", ContainerDeclaration.to_json container);
            ( "inheritedMembers",
              JSON_Array
                (Base.List.map ~f:MemberCluster.to_json inheritedMembers) );
          ]
    end

    include Key
    include Extend (Key)
  end

  module CallArgument = struct
    type t = {
      span: Src.RelByteSpan.t;
      argument: Argument.t option;
    }
    [@@deriving ord]

    let to_json { span; argument } =
      let fields = [("span", Src.RelByteSpan.to_json span)] in
      let fields =
        match argument with
        | Some argument -> ("argument", Argument.to_json argument) :: fields
        | None -> fields
      in
      JSON_Object fields
  end

  module ModuleMembership = struct
    type t = {
      declaration: ModuleDeclaration.t;
      internal: bool;
    }
    [@@deriving ord]

    let to_json { declaration; internal } =
      JSON_Object
        [
          ("declaration", ModuleDeclaration.to_json declaration);
          ("internal", JSON_Bool internal);
        ]
  end

  module Signature = struct
    module Key = struct
      type key = {
        returns: Type.t option;
        parameters: Parameter.t list;
        contexts: Context.t list option;
        returnsTypeInfo: TypeInfo.t option;
      }
      [@@deriving ord]

      let to_json_key { returns; parameters; contexts; returnsTypeInfo } =
        let fields =
          [
            ("parameters", JSON_Array (List.map ~f:Parameter.to_json parameters));
          ]
        in
        let fields =
          match returns with
          | None -> fields
          | Some returns -> ("returns", Type.to_json returns) :: fields
        in
        let fields =
          match returnsTypeInfo with
          | None -> fields
          | Some returnsTypeInfo ->
            ("returnsTypeInfo", TypeInfo.to_json returnsTypeInfo) :: fields
        in
        let fields =
          match contexts with
          | None -> fields
          | Some contexts ->
            ("contexts", JSON_Array (List.map ~f:Context.to_json contexts))
            :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module FileCall = struct
    module Key = struct
      type key = {
        file: Src.File.t;
        callee_span: Src.ByteSpan.t;
        callee_xref: XRefTarget.t option;
        dispatch_arg: CallArgument.t option;
        receiver_type: Declaration.t option;
        call_args: CallArgument.t list;
        callee_xrefs: XRefTarget.t list;
      }
      [@@deriving ord]

      let to_json_key
          {
            file;
            callee_span;
            callee_xref;
            dispatch_arg;
            receiver_type;
            call_args;
            callee_xrefs;
          } =
        let fields =
          [
            ("file", Src.File.to_json file);
            ("callee_span", Src.ByteSpan.to_json callee_span);
            ( "call_args",
              JSON_Array (List.map call_args ~f:CallArgument.to_json) );
            ( "callee_xrefs",
              JSON_Array (List.map callee_xrefs ~f:XRefTarget.to_json) );
          ]
        in
        let fields =
          match callee_xref with
          | Some callee_xref ->
            ("callee_xref", XRefTarget.to_json callee_xref) :: fields
          | None -> fields
        in
        let fields =
          match receiver_type with
          | Some receiver_type ->
            ("receiver_type", Declaration.to_json receiver_type) :: fields
          | None -> fields
        in
        let fields =
          match dispatch_arg with
          | Some dispatch_arg ->
            ("dispatch_arg", CallArgument.to_json dispatch_arg) :: fields
          | None -> fields
        in

        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module GlobalNamespaceAlias = struct
    module Key = struct
      type key = {
        from: Name.t;
        to_: NamespaceQName.t;
      }
      [@@deriving ord]

      let to_json_key { from; to_ } =
        let fields =
          [("from", Name.to_json from); ("to", NamespaceQName.to_json to_)]
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module DeclarationSpan = struct
    module Key = struct
      type key = {
        declaration: Declaration.t;
        file: Src.File.t;
        span: Src.ByteSpan.t;
      }
      [@@deriving ord]

      let to_json_key { declaration; file; span } =
        let fields =
          [
            ("declaration", Declaration.to_json declaration);
            ("file", Src.File.to_json file);
            ("span", Src.ByteSpan.to_json span);
          ]
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module DeclarationComment = struct
    module Key = struct
      type key = {
        declaration: Declaration.t;
        file: Src.File.t;
        span: Src.ByteSpan.t;
      }
      [@@deriving ord]

      let to_json_key { declaration; file; span } =
        let fields =
          [
            ("declaration", Declaration.to_json declaration);
            ("file", Src.File.to_json file);
            ("span", Src.ByteSpan.to_json span);
          ]
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module ModuleDefinition = struct
    module Key = struct
      type key = {
        declaration: ModuleDeclaration.t;
        attributes: UserAttribute.t list;
      }
      [@@deriving ord]

      let to_json_key { declaration; attributes } =
        let fields =
          [
            ("declaration", ModuleDeclaration.to_json declaration);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
          ]
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module ClassDefinition = struct
    module Key = struct
      type key = {
        declaration: ClassDeclaration.t;
        isAbstract: bool;
        isFinal: bool;
        members: Declaration.t list;
        extends_: ClassDeclaration.t option;
        implements_: InterfaceDeclaration.t list;
        uses: TraitDeclaration.t list;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        module_: ModuleMembership.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            isAbstract;
            isFinal;
            members;
            extends_;
            implements_;
            uses;
            attributes;
            typeParams;
            module_;
          } =
        let fields =
          [
            ("declaration", ClassDeclaration.to_json declaration);
            ("isAbstract", JSON_Bool isAbstract);
            ("isFinal", JSON_Bool isFinal);
            ("members", JSON_Array (List.map ~f:Declaration.to_json members));
            ( "implements_",
              JSON_Array (List.map ~f:InterfaceDeclaration.to_json implements_)
            );
            ("uses", JSON_Array (List.map ~f:TraitDeclaration.to_json uses));
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
          ]
        in
        let fields =
          match extends_ with
          | None -> fields
          | Some extends_ ->
            ("extends_", ClassDeclaration.to_json extends_) :: fields
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module InterfaceDefinition = struct
    module Key = struct
      type key = {
        declaration: InterfaceDeclaration.t;
        members: Declaration.t list;
        extends_: InterfaceDeclaration.t list;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        requireExtends: ClassDeclaration.t list;
        module_: ModuleMembership.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            members;
            extends_;
            attributes;
            typeParams;
            requireExtends;
            module_;
          } =
        let fields =
          [
            ("declaration", InterfaceDeclaration.to_json declaration);
            ("members", JSON_Array (List.map ~f:Declaration.to_json members));
            ( "extends_",
              JSON_Array (List.map ~f:InterfaceDeclaration.to_json extends_) );
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
            ( "requireExtends",
              JSON_Array (List.map ~f:ClassDeclaration.to_json requireExtends)
            );
          ]
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module TraitDefinition = struct
    module Key = struct
      type key = {
        declaration: TraitDeclaration.t;
        members: Declaration.t list;
        implements_: InterfaceDeclaration.t list;
        uses: TraitDeclaration.t list;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        requireExtends: ClassDeclaration.t list;
        requireImplements: InterfaceDeclaration.t list;
        requireClass: ClassDeclaration.t list option;
        module_: ModuleMembership.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            members;
            implements_;
            uses;
            attributes;
            typeParams;
            requireExtends;
            requireImplements;
            module_;
            requireClass;
          } =
        let fields =
          [
            ("declaration", TraitDeclaration.to_json declaration);
            ("members", JSON_Array (List.map ~f:Declaration.to_json members));
            ( "implements_",
              JSON_Array (List.map ~f:InterfaceDeclaration.to_json implements_)
            );
            ("uses", JSON_Array (List.map ~f:TraitDeclaration.to_json uses));
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
            ( "requireExtends",
              JSON_Array (List.map ~f:ClassDeclaration.to_json requireExtends)
            );
            ( "requireImplements",
              JSON_Array
                (List.map ~f:InterfaceDeclaration.to_json requireImplements) );
          ]
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        let fields =
          match requireClass with
          | None -> fields
          | Some requireClass ->
            ( "requireClass",
              JSON_Array (List.map ~f:ClassDeclaration.to_json requireClass) )
            :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module EnumDefinition = struct
    module Key = struct
      type key = {
        declaration: EnumDeclaration.t;
        enumBase: Type.t;
        enumConstraint: Type.t option;
        enumerators: Enumerator.t list;
        attributes: UserAttribute.t list;
        includes: EnumDeclaration.t list;
        isEnumClass: bool;
        module_: ModuleMembership.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            enumBase;
            enumConstraint;
            enumerators;
            attributes;
            includes;
            isEnumClass;
            module_;
          } =
        let fields =
          [
            ("declaration", EnumDeclaration.to_json declaration);
            ("enumBase", Type.to_json enumBase);
            ( "enumerators",
              JSON_Array (List.map ~f:Enumerator.to_json enumerators) );
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "includes",
              JSON_Array (List.map ~f:EnumDeclaration.to_json includes) );
            ("isEnumClass", JSON_Bool isEnumClass);
          ]
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        let fields =
          match enumConstraint with
          | None -> fields
          | Some enumConstraint ->
            ("enumConstraint", Type.to_json enumConstraint) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module TypedefDefinition = struct
    module Key = struct
      type key = {
        declaration: TypedefDeclaration.t;
        isTransparent: bool;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        module_: ModuleMembership.t option;
      }
      [@@deriving ord]

      let to_json_key
          { declaration; isTransparent; attributes; typeParams; module_ } =
        let fields =
          [
            ("declaration", TypedefDeclaration.to_json declaration);
            ("isTransparent", JSON_Bool isTransparent);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
          ]
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module MethodDefinition = struct
    module Key = struct
      type key = {
        declaration: MethodDeclaration.t;
        signature: Signature.t;
        visibility: Visibility.t;
        isAbstract: bool;
        isAsync: bool;
        isFinal: bool;
        isStatic: bool;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        isReadonlyThis: bool option;
        readonlyRet: ReadonlyKind.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            signature;
            visibility;
            isAbstract;
            isAsync;
            isFinal;
            isStatic;
            attributes;
            typeParams;
            isReadonlyThis;
            readonlyRet;
          } =
        let fields =
          [
            ("declaration", MethodDeclaration.to_json declaration);
            ("signature", Signature.to_json signature);
            ("visibility", Visibility.to_json visibility);
            ("isAbstract", JSON_Bool isAbstract);
            ("isAsync", JSON_Bool isAsync);
            ("isFinal", JSON_Bool isFinal);
            ("isStatic", JSON_Bool isStatic);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
          ]
        in
        let fields =
          match isReadonlyThis with
          | None -> fields
          | Some isReadonlyThis ->
            ("isReadonlyThis", JSON_Bool isReadonlyThis) :: fields
        in
        let fields =
          match readonlyRet with
          | None -> fields
          | Some readonlyRet ->
            ("readonlyRet", ReadonlyKind.to_json readonlyRet) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module GlobalConstDefinition = struct
    module Key = struct
      type key = {
        declaration: GlobalConstDeclaration.t;
        type_: Type.t option;
        value: string;
      }
      [@@deriving ord]

      let to_json_key { declaration; type_; value } =
        let fields =
          [
            ("declaration", GlobalConstDeclaration.to_json declaration);
            ("value", JSON_String value);
          ]
        in
        let fields =
          match type_ with
          | None -> fields
          | Some type_ -> ("type", Type.to_json type_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module ClassConstDefinition = struct
    module Key = struct
      type key = {
        declaration: ClassConstDeclaration.t;
        type_: Type.t option;
        value: string option;
      }
      [@@deriving ord]

      let to_json_key { declaration; type_; value } =
        let fields =
          [("declaration", ClassConstDeclaration.to_json declaration)]
        in
        let fields =
          match type_ with
          | None -> fields
          | Some type_ -> ("type", Type.to_json type_) :: fields
        in
        let fields =
          match value with
          | None -> fields
          | Some value -> ("value", Hh_json.JSON_String value) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module TypeConstDefinition = struct
    module Key = struct
      type key = {
        declaration: TypeConstDeclaration.t;
        type_: Type.t option;
        kind: TypeConstKind.t;
        attributes: UserAttribute.t list;
      }
      [@@deriving ord]

      let to_json_key { declaration; type_; kind; attributes } =
        let fields =
          [
            ("declaration", TypeConstDeclaration.to_json declaration);
            ("kind", TypeConstKind.to_json kind);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
          ]
        in
        let fields =
          match type_ with
          | None -> fields
          | Some type_ -> ("type", Type.to_json type_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module FunctionDefinition = struct
    module Key = struct
      type key = {
        declaration: FunctionDeclaration.t;
        signature: Signature.t;
        isAsync: bool;
        attributes: UserAttribute.t list;
        typeParams: TypeParameter.t list;
        module_: ModuleMembership.t option;
        readonlyRet: ReadonlyKind.t option;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            signature;
            isAsync;
            attributes;
            typeParams;
            module_;
            readonlyRet;
          } =
        let fields =
          [
            ("declaration", FunctionDeclaration.to_json declaration);
            ("signature", Signature.to_json signature);
            ("isAsync", JSON_Bool isAsync);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
            ( "typeParams",
              JSON_Array (List.map ~f:TypeParameter.to_json typeParams) );
          ]
        in
        let fields =
          match module_ with
          | None -> fields
          | Some module_ ->
            ("module_", ModuleMembership.to_json module_) :: fields
        in
        let fields =
          match readonlyRet with
          | None -> fields
          | Some readonly_ret ->
            ("readonlyRet", ReadonlyKind.to_json readonly_ret) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module PropertyDefinition = struct
    module Key = struct
      type key = {
        declaration: PropertyDeclaration.t;
        type_: Type.t option;
        visibility: Visibility.t;
        isFinal: bool;
        isAbstract: bool;
        isStatic: bool;
        attributes: UserAttribute.t list;
      }
      [@@deriving ord]

      let to_json_key
          {
            declaration;
            type_;
            visibility;
            isFinal;
            isAbstract;
            isStatic;
            attributes;
          } =
        let fields =
          [
            ("declaration", PropertyDeclaration.to_json declaration);
            ("visibility", Visibility.to_json visibility);
            ("isFinal", JSON_Bool isFinal);
            ("isAbstract", JSON_Bool isAbstract);
            ("isStatic", JSON_Bool isStatic);
            ( "attributes",
              JSON_Array (List.map ~f:UserAttribute.to_json attributes) );
          ]
        in
        let fields =
          match type_ with
          | None -> fields
          | Some type_ -> ("type", Type.to_json type_) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end

  module IndexerInputHash = struct
    module Key = struct
      type key = string [@@deriving ord]

      type value = string list

      let to_json_key s = JSON_String s

      let to_json_value v =
        JSON_String (List.map v ~f:Base64.encode_string |> String.concat)
    end

    include Key
    include Extend (Key)
  end
end

module GenCode = struct
  module GenCodeCommand = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k = JSON_String k
    end

    include Key
    include Extend (Key)
  end

  module GenCodeClass = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k = JSON_String k
    end

    include Key
    include Extend (Key)
  end

  module GenCodeSignature = struct
    module Key = struct
      type key = string [@@deriving ord]

      let to_json_key k = JSON_String k
    end

    include Key
    include Extend (Key)
  end

  module GenCodeVariant = struct
    type t =
      | Full
      | Partial
    [@@deriving ord]

    let to_json t =
      let num =
        match t with
        | Full -> 0
        | Partial -> 1
      in
      JSON_Number (string_of_int num)
  end

  module GenCode = struct
    module Key = struct
      type key = {
        file: Src.File.t;
        variant: GenCodeVariant.t;
        source: Src.File.t option;
        command: GenCodeCommand.t option;
        class_: GenCodeClass.t option;
        signature: GenCodeSignature.t option;
      }
      [@@deriving ord]

      let to_json_key { file; variant; source; command; class_; signature } =
        let fields =
          [
            ("file", Src.File.to_json file);
            ("variant", GenCodeVariant.to_json variant);
          ]
        in
        let fields =
          match source with
          | None -> fields
          | Some source -> ("source", Src.File.to_json source) :: fields
        in
        let fields =
          match command with
          | None -> fields
          | Some command ->
            ("command", GenCodeCommand.to_json command) :: fields
        in
        let fields =
          match class_ with
          | None -> fields
          | Some class_ -> ("class_", GenCodeClass.to_json class_) :: fields
        in
        let fields =
          match signature with
          | None -> fields
          | Some signature ->
            ("signature", GenCodeSignature.to_json signature) :: fields
        in
        JSON_Object fields
    end

    include Key
    include Extend (Key)
  end
end
