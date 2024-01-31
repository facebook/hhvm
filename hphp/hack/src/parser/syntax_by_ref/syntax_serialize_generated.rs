/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */
use super::{serialize::WithContext, syntax::Syntax, syntax_variant_generated::*};
use serde::{ser::SerializeStruct, Serialize, Serializer};

impl<'a, T, V> Serialize for WithContext<'a, Syntax<'a, T, V>>
where
    T: 'a,
    WithContext<'a, T>: Serialize,
{
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        match self.1.children {
            SyntaxVariant::Missing => {
                let mut ss = s.serialize_struct("", 1)?;
                ss.serialize_field("kind", "missing")?;
                ss.end()
            }
            SyntaxVariant::Token(ref t) => {
                let mut ss = s.serialize_struct("", 2)?;
                ss.serialize_field("kind", "token")?;
                ss.serialize_field("token", &self.with(t))?;
                ss.end()
            }
            SyntaxVariant::SyntaxList(l) => {
                let mut ss = s.serialize_struct("", 2)?;
                ss.serialize_field("kind", "list")?;
                ss.serialize_field("elements", &self.with(l))?;
                ss.end()
            }
            SyntaxVariant::EndOfFile (EndOfFileChildren{token} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "end_of_file")?;
      ss.serialize_field("end_of_file_token", &self.with(token))?;
      ss.end()
} 
SyntaxVariant::Script (ScriptChildren{declarations} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "script")?;
      ss.serialize_field("script_declarations", &self.with(declarations))?;
      ss.end()
} 
SyntaxVariant::QualifiedName (QualifiedNameChildren{parts} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "qualified_name")?;
      ss.serialize_field("qualified_name_parts", &self.with(parts))?;
      ss.end()
} 
SyntaxVariant::ModuleName (ModuleNameChildren{parts} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "module_name")?;
      ss.serialize_field("module_name_parts", &self.with(parts))?;
      ss.end()
} 
SyntaxVariant::SimpleTypeSpecifier (SimpleTypeSpecifierChildren{specifier} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "simple_type_specifier")?;
      ss.serialize_field("simple_type_specifier", &self.with(specifier))?;
      ss.end()
} 
SyntaxVariant::LiteralExpression (LiteralExpressionChildren{expression} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "literal")?;
      ss.serialize_field("literal_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::PrefixedStringExpression (PrefixedStringExpressionChildren{name,str} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "prefixed_string")?;
      ss.serialize_field("prefixed_string_name", &self.with(name))?;
ss.serialize_field("prefixed_string_str", &self.with(str))?;
      ss.end()
} 
SyntaxVariant::PrefixedCodeExpression (PrefixedCodeExpressionChildren{prefix,left_backtick,body,right_backtick} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "prefixed_code")?;
      ss.serialize_field("prefixed_code_prefix", &self.with(prefix))?;
ss.serialize_field("prefixed_code_left_backtick", &self.with(left_backtick))?;
ss.serialize_field("prefixed_code_body", &self.with(body))?;
ss.serialize_field("prefixed_code_right_backtick", &self.with(right_backtick))?;
      ss.end()
} 
SyntaxVariant::VariableExpression (VariableExpressionChildren{expression} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "variable")?;
      ss.serialize_field("variable_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::PipeVariableExpression (PipeVariableExpressionChildren{expression} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "pipe_variable")?;
      ss.serialize_field("pipe_variable_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::FileAttributeSpecification (FileAttributeSpecificationChildren{left_double_angle,keyword,colon,attributes,right_double_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "file_attribute_specification")?;
      ss.serialize_field("file_attribute_specification_left_double_angle", &self.with(left_double_angle))?;
ss.serialize_field("file_attribute_specification_keyword", &self.with(keyword))?;
ss.serialize_field("file_attribute_specification_colon", &self.with(colon))?;
ss.serialize_field("file_attribute_specification_attributes", &self.with(attributes))?;
ss.serialize_field("file_attribute_specification_right_double_angle", &self.with(right_double_angle))?;
      ss.end()
} 
SyntaxVariant::EnumDeclaration (EnumDeclarationChildren{attribute_spec,modifiers,keyword,name,colon,base,type_,left_brace,use_clauses,enumerators,right_brace} ) => {
      let mut ss = s.serialize_struct("", 12)?;
      ss.serialize_field("kind", "enum_declaration")?;
      ss.serialize_field("enum_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("enum_modifiers", &self.with(modifiers))?;
ss.serialize_field("enum_keyword", &self.with(keyword))?;
ss.serialize_field("enum_name", &self.with(name))?;
ss.serialize_field("enum_colon", &self.with(colon))?;
ss.serialize_field("enum_base", &self.with(base))?;
ss.serialize_field("enum_type", &self.with(type_))?;
ss.serialize_field("enum_left_brace", &self.with(left_brace))?;
ss.serialize_field("enum_use_clauses", &self.with(use_clauses))?;
ss.serialize_field("enum_enumerators", &self.with(enumerators))?;
ss.serialize_field("enum_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::EnumUse (EnumUseChildren{keyword,names,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "enum_use")?;
      ss.serialize_field("enum_use_keyword", &self.with(keyword))?;
ss.serialize_field("enum_use_names", &self.with(names))?;
ss.serialize_field("enum_use_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::Enumerator (EnumeratorChildren{name,equal,value,semicolon} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "enumerator")?;
      ss.serialize_field("enumerator_name", &self.with(name))?;
ss.serialize_field("enumerator_equal", &self.with(equal))?;
ss.serialize_field("enumerator_value", &self.with(value))?;
ss.serialize_field("enumerator_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::EnumClassDeclaration (EnumClassDeclarationChildren{attribute_spec,modifiers,enum_keyword,class_keyword,name,colon,base,extends,extends_list,left_brace,elements,right_brace} ) => {
      let mut ss = s.serialize_struct("", 13)?;
      ss.serialize_field("kind", "enum_class_declaration")?;
      ss.serialize_field("enum_class_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("enum_class_modifiers", &self.with(modifiers))?;
ss.serialize_field("enum_class_enum_keyword", &self.with(enum_keyword))?;
ss.serialize_field("enum_class_class_keyword", &self.with(class_keyword))?;
ss.serialize_field("enum_class_name", &self.with(name))?;
ss.serialize_field("enum_class_colon", &self.with(colon))?;
ss.serialize_field("enum_class_base", &self.with(base))?;
ss.serialize_field("enum_class_extends", &self.with(extends))?;
ss.serialize_field("enum_class_extends_list", &self.with(extends_list))?;
ss.serialize_field("enum_class_left_brace", &self.with(left_brace))?;
ss.serialize_field("enum_class_elements", &self.with(elements))?;
ss.serialize_field("enum_class_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::EnumClassEnumerator (EnumClassEnumeratorChildren{modifiers,type_,name,initializer,semicolon} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "enum_class_enumerator")?;
      ss.serialize_field("enum_class_enumerator_modifiers", &self.with(modifiers))?;
ss.serialize_field("enum_class_enumerator_type", &self.with(type_))?;
ss.serialize_field("enum_class_enumerator_name", &self.with(name))?;
ss.serialize_field("enum_class_enumerator_initializer", &self.with(initializer))?;
ss.serialize_field("enum_class_enumerator_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::AliasDeclaration (AliasDeclarationChildren{attribute_spec,modifiers,module_kw_opt,keyword,name,generic_parameter,constraint,equal,type_,semicolon} ) => {
      let mut ss = s.serialize_struct("", 11)?;
      ss.serialize_field("kind", "alias_declaration")?;
      ss.serialize_field("alias_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("alias_modifiers", &self.with(modifiers))?;
ss.serialize_field("alias_module_kw_opt", &self.with(module_kw_opt))?;
ss.serialize_field("alias_keyword", &self.with(keyword))?;
ss.serialize_field("alias_name", &self.with(name))?;
ss.serialize_field("alias_generic_parameter", &self.with(generic_parameter))?;
ss.serialize_field("alias_constraint", &self.with(constraint))?;
ss.serialize_field("alias_equal", &self.with(equal))?;
ss.serialize_field("alias_type", &self.with(type_))?;
ss.serialize_field("alias_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ContextAliasDeclaration (ContextAliasDeclarationChildren{attribute_spec,keyword,name,generic_parameter,as_constraint,equal,context,semicolon} ) => {
      let mut ss = s.serialize_struct("", 9)?;
      ss.serialize_field("kind", "context_alias_declaration")?;
      ss.serialize_field("ctx_alias_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("ctx_alias_keyword", &self.with(keyword))?;
ss.serialize_field("ctx_alias_name", &self.with(name))?;
ss.serialize_field("ctx_alias_generic_parameter", &self.with(generic_parameter))?;
ss.serialize_field("ctx_alias_as_constraint", &self.with(as_constraint))?;
ss.serialize_field("ctx_alias_equal", &self.with(equal))?;
ss.serialize_field("ctx_alias_context", &self.with(context))?;
ss.serialize_field("ctx_alias_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::CaseTypeDeclaration (CaseTypeDeclarationChildren{attribute_spec,modifiers,case_keyword,type_keyword,name,generic_parameter,as_,bounds,equal,variants,semicolon} ) => {
      let mut ss = s.serialize_struct("", 12)?;
      ss.serialize_field("kind", "case_type_declaration")?;
      ss.serialize_field("case_type_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("case_type_modifiers", &self.with(modifiers))?;
ss.serialize_field("case_type_case_keyword", &self.with(case_keyword))?;
ss.serialize_field("case_type_type_keyword", &self.with(type_keyword))?;
ss.serialize_field("case_type_name", &self.with(name))?;
ss.serialize_field("case_type_generic_parameter", &self.with(generic_parameter))?;
ss.serialize_field("case_type_as", &self.with(as_))?;
ss.serialize_field("case_type_bounds", &self.with(bounds))?;
ss.serialize_field("case_type_equal", &self.with(equal))?;
ss.serialize_field("case_type_variants", &self.with(variants))?;
ss.serialize_field("case_type_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::CaseTypeVariant (CaseTypeVariantChildren{bar,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "case_type_variant")?;
      ss.serialize_field("case_type_variant_bar", &self.with(bar))?;
ss.serialize_field("case_type_variant_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::PropertyDeclaration (PropertyDeclarationChildren{attribute_spec,modifiers,type_,declarators,semicolon} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "property_declaration")?;
      ss.serialize_field("property_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("property_modifiers", &self.with(modifiers))?;
ss.serialize_field("property_type", &self.with(type_))?;
ss.serialize_field("property_declarators", &self.with(declarators))?;
ss.serialize_field("property_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::PropertyDeclarator (PropertyDeclaratorChildren{name,initializer} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "property_declarator")?;
      ss.serialize_field("property_name", &self.with(name))?;
ss.serialize_field("property_initializer", &self.with(initializer))?;
      ss.end()
} 
SyntaxVariant::NamespaceDeclaration (NamespaceDeclarationChildren{header,body} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "namespace_declaration")?;
      ss.serialize_field("namespace_header", &self.with(header))?;
ss.serialize_field("namespace_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::NamespaceDeclarationHeader (NamespaceDeclarationHeaderChildren{keyword,name} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "namespace_declaration_header")?;
      ss.serialize_field("namespace_keyword", &self.with(keyword))?;
ss.serialize_field("namespace_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::NamespaceBody (NamespaceBodyChildren{left_brace,declarations,right_brace} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "namespace_body")?;
      ss.serialize_field("namespace_left_brace", &self.with(left_brace))?;
ss.serialize_field("namespace_declarations", &self.with(declarations))?;
ss.serialize_field("namespace_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::NamespaceEmptyBody (NamespaceEmptyBodyChildren{semicolon} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "namespace_empty_body")?;
      ss.serialize_field("namespace_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::NamespaceUseDeclaration (NamespaceUseDeclarationChildren{keyword,kind,clauses,semicolon} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "namespace_use_declaration")?;
      ss.serialize_field("namespace_use_keyword", &self.with(keyword))?;
ss.serialize_field("namespace_use_kind", &self.with(kind))?;
ss.serialize_field("namespace_use_clauses", &self.with(clauses))?;
ss.serialize_field("namespace_use_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::NamespaceGroupUseDeclaration (NamespaceGroupUseDeclarationChildren{keyword,kind,prefix,left_brace,clauses,right_brace,semicolon} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "namespace_group_use_declaration")?;
      ss.serialize_field("namespace_group_use_keyword", &self.with(keyword))?;
ss.serialize_field("namespace_group_use_kind", &self.with(kind))?;
ss.serialize_field("namespace_group_use_prefix", &self.with(prefix))?;
ss.serialize_field("namespace_group_use_left_brace", &self.with(left_brace))?;
ss.serialize_field("namespace_group_use_clauses", &self.with(clauses))?;
ss.serialize_field("namespace_group_use_right_brace", &self.with(right_brace))?;
ss.serialize_field("namespace_group_use_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::NamespaceUseClause (NamespaceUseClauseChildren{clause_kind,name,as_,alias} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "namespace_use_clause")?;
      ss.serialize_field("namespace_use_clause_kind", &self.with(clause_kind))?;
ss.serialize_field("namespace_use_name", &self.with(name))?;
ss.serialize_field("namespace_use_as", &self.with(as_))?;
ss.serialize_field("namespace_use_alias", &self.with(alias))?;
      ss.end()
} 
SyntaxVariant::FunctionDeclaration (FunctionDeclarationChildren{attribute_spec,declaration_header,body} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "function_declaration")?;
      ss.serialize_field("function_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("function_declaration_header", &self.with(declaration_header))?;
ss.serialize_field("function_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::FunctionDeclarationHeader (FunctionDeclarationHeaderChildren{modifiers,keyword,name,type_parameter_list,left_paren,parameter_list,right_paren,contexts,colon,readonly_return,type_,where_clause} ) => {
      let mut ss = s.serialize_struct("", 13)?;
      ss.serialize_field("kind", "function_declaration_header")?;
      ss.serialize_field("function_modifiers", &self.with(modifiers))?;
ss.serialize_field("function_keyword", &self.with(keyword))?;
ss.serialize_field("function_name", &self.with(name))?;
ss.serialize_field("function_type_parameter_list", &self.with(type_parameter_list))?;
ss.serialize_field("function_left_paren", &self.with(left_paren))?;
ss.serialize_field("function_parameter_list", &self.with(parameter_list))?;
ss.serialize_field("function_right_paren", &self.with(right_paren))?;
ss.serialize_field("function_contexts", &self.with(contexts))?;
ss.serialize_field("function_colon", &self.with(colon))?;
ss.serialize_field("function_readonly_return", &self.with(readonly_return))?;
ss.serialize_field("function_type", &self.with(type_))?;
ss.serialize_field("function_where_clause", &self.with(where_clause))?;
      ss.end()
} 
SyntaxVariant::Contexts (ContextsChildren{left_bracket,types,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "contexts")?;
      ss.serialize_field("contexts_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("contexts_types", &self.with(types))?;
ss.serialize_field("contexts_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::WhereClause (WhereClauseChildren{keyword,constraints} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "where_clause")?;
      ss.serialize_field("where_clause_keyword", &self.with(keyword))?;
ss.serialize_field("where_clause_constraints", &self.with(constraints))?;
      ss.end()
} 
SyntaxVariant::WhereConstraint (WhereConstraintChildren{left_type,operator,right_type} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "where_constraint")?;
      ss.serialize_field("where_constraint_left_type", &self.with(left_type))?;
ss.serialize_field("where_constraint_operator", &self.with(operator))?;
ss.serialize_field("where_constraint_right_type", &self.with(right_type))?;
      ss.end()
} 
SyntaxVariant::MethodishDeclaration (MethodishDeclarationChildren{attribute,function_decl_header,function_body,semicolon} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "methodish_declaration")?;
      ss.serialize_field("methodish_attribute", &self.with(attribute))?;
ss.serialize_field("methodish_function_decl_header", &self.with(function_decl_header))?;
ss.serialize_field("methodish_function_body", &self.with(function_body))?;
ss.serialize_field("methodish_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::MethodishTraitResolution (MethodishTraitResolutionChildren{attribute,function_decl_header,equal,name,semicolon} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "methodish_trait_resolution")?;
      ss.serialize_field("methodish_trait_attribute", &self.with(attribute))?;
ss.serialize_field("methodish_trait_function_decl_header", &self.with(function_decl_header))?;
ss.serialize_field("methodish_trait_equal", &self.with(equal))?;
ss.serialize_field("methodish_trait_name", &self.with(name))?;
ss.serialize_field("methodish_trait_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ClassishDeclaration (ClassishDeclarationChildren{attribute,modifiers,xhp,keyword,name,type_parameters,extends_keyword,extends_list,implements_keyword,implements_list,where_clause,body} ) => {
      let mut ss = s.serialize_struct("", 13)?;
      ss.serialize_field("kind", "classish_declaration")?;
      ss.serialize_field("classish_attribute", &self.with(attribute))?;
ss.serialize_field("classish_modifiers", &self.with(modifiers))?;
ss.serialize_field("classish_xhp", &self.with(xhp))?;
ss.serialize_field("classish_keyword", &self.with(keyword))?;
ss.serialize_field("classish_name", &self.with(name))?;
ss.serialize_field("classish_type_parameters", &self.with(type_parameters))?;
ss.serialize_field("classish_extends_keyword", &self.with(extends_keyword))?;
ss.serialize_field("classish_extends_list", &self.with(extends_list))?;
ss.serialize_field("classish_implements_keyword", &self.with(implements_keyword))?;
ss.serialize_field("classish_implements_list", &self.with(implements_list))?;
ss.serialize_field("classish_where_clause", &self.with(where_clause))?;
ss.serialize_field("classish_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::ClassishBody (ClassishBodyChildren{left_brace,elements,right_brace} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "classish_body")?;
      ss.serialize_field("classish_body_left_brace", &self.with(left_brace))?;
ss.serialize_field("classish_body_elements", &self.with(elements))?;
ss.serialize_field("classish_body_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::TraitUse (TraitUseChildren{keyword,names,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "trait_use")?;
      ss.serialize_field("trait_use_keyword", &self.with(keyword))?;
ss.serialize_field("trait_use_names", &self.with(names))?;
ss.serialize_field("trait_use_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::RequireClause (RequireClauseChildren{keyword,kind,name,semicolon} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "require_clause")?;
      ss.serialize_field("require_keyword", &self.with(keyword))?;
ss.serialize_field("require_kind", &self.with(kind))?;
ss.serialize_field("require_name", &self.with(name))?;
ss.serialize_field("require_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ConstDeclaration (ConstDeclarationChildren{attribute_spec,modifiers,keyword,type_specifier,declarators,semicolon} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "const_declaration")?;
      ss.serialize_field("const_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("const_modifiers", &self.with(modifiers))?;
ss.serialize_field("const_keyword", &self.with(keyword))?;
ss.serialize_field("const_type_specifier", &self.with(type_specifier))?;
ss.serialize_field("const_declarators", &self.with(declarators))?;
ss.serialize_field("const_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ConstantDeclarator (ConstantDeclaratorChildren{name,initializer} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "constant_declarator")?;
      ss.serialize_field("constant_declarator_name", &self.with(name))?;
ss.serialize_field("constant_declarator_initializer", &self.with(initializer))?;
      ss.end()
} 
SyntaxVariant::TypeConstDeclaration (TypeConstDeclarationChildren{attribute_spec,modifiers,keyword,type_keyword,name,type_parameters,type_constraints,equal,type_specifier,semicolon} ) => {
      let mut ss = s.serialize_struct("", 11)?;
      ss.serialize_field("kind", "type_const_declaration")?;
      ss.serialize_field("type_const_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("type_const_modifiers", &self.with(modifiers))?;
ss.serialize_field("type_const_keyword", &self.with(keyword))?;
ss.serialize_field("type_const_type_keyword", &self.with(type_keyword))?;
ss.serialize_field("type_const_name", &self.with(name))?;
ss.serialize_field("type_const_type_parameters", &self.with(type_parameters))?;
ss.serialize_field("type_const_type_constraints", &self.with(type_constraints))?;
ss.serialize_field("type_const_equal", &self.with(equal))?;
ss.serialize_field("type_const_type_specifier", &self.with(type_specifier))?;
ss.serialize_field("type_const_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ContextConstDeclaration (ContextConstDeclarationChildren{modifiers,const_keyword,ctx_keyword,name,type_parameters,constraint,equal,ctx_list,semicolon} ) => {
      let mut ss = s.serialize_struct("", 10)?;
      ss.serialize_field("kind", "context_const_declaration")?;
      ss.serialize_field("context_const_modifiers", &self.with(modifiers))?;
ss.serialize_field("context_const_const_keyword", &self.with(const_keyword))?;
ss.serialize_field("context_const_ctx_keyword", &self.with(ctx_keyword))?;
ss.serialize_field("context_const_name", &self.with(name))?;
ss.serialize_field("context_const_type_parameters", &self.with(type_parameters))?;
ss.serialize_field("context_const_constraint", &self.with(constraint))?;
ss.serialize_field("context_const_equal", &self.with(equal))?;
ss.serialize_field("context_const_ctx_list", &self.with(ctx_list))?;
ss.serialize_field("context_const_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::DecoratedExpression (DecoratedExpressionChildren{decorator,expression} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "decorated_expression")?;
      ss.serialize_field("decorated_expression_decorator", &self.with(decorator))?;
ss.serialize_field("decorated_expression_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::ParameterDeclaration (ParameterDeclarationChildren{attribute,visibility,call_convention,readonly,type_,name,default_value,parameter_end} ) => {
      let mut ss = s.serialize_struct("", 9)?;
      ss.serialize_field("kind", "parameter_declaration")?;
      ss.serialize_field("parameter_attribute", &self.with(attribute))?;
ss.serialize_field("parameter_visibility", &self.with(visibility))?;
ss.serialize_field("parameter_call_convention", &self.with(call_convention))?;
ss.serialize_field("parameter_readonly", &self.with(readonly))?;
ss.serialize_field("parameter_type", &self.with(type_))?;
ss.serialize_field("parameter_name", &self.with(name))?;
ss.serialize_field("parameter_default_value", &self.with(default_value))?;
ss.serialize_field("parameter_parameter_end", &self.with(parameter_end))?;
      ss.end()
} 
SyntaxVariant::VariadicParameter (VariadicParameterChildren{call_convention,type_,ellipsis} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "variadic_parameter")?;
      ss.serialize_field("variadic_parameter_call_convention", &self.with(call_convention))?;
ss.serialize_field("variadic_parameter_type", &self.with(type_))?;
ss.serialize_field("variadic_parameter_ellipsis", &self.with(ellipsis))?;
      ss.end()
} 
SyntaxVariant::OldAttributeSpecification (OldAttributeSpecificationChildren{left_double_angle,attributes,right_double_angle} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "old_attribute_specification")?;
      ss.serialize_field("old_attribute_specification_left_double_angle", &self.with(left_double_angle))?;
ss.serialize_field("old_attribute_specification_attributes", &self.with(attributes))?;
ss.serialize_field("old_attribute_specification_right_double_angle", &self.with(right_double_angle))?;
      ss.end()
} 
SyntaxVariant::AttributeSpecification (AttributeSpecificationChildren{attributes} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "attribute_specification")?;
      ss.serialize_field("attribute_specification_attributes", &self.with(attributes))?;
      ss.end()
} 
SyntaxVariant::Attribute (AttributeChildren{at,attribute_name} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "attribute")?;
      ss.serialize_field("attribute_at", &self.with(at))?;
ss.serialize_field("attribute_attribute_name", &self.with(attribute_name))?;
      ss.end()
} 
SyntaxVariant::InclusionExpression (InclusionExpressionChildren{require,filename} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "inclusion_expression")?;
      ss.serialize_field("inclusion_require", &self.with(require))?;
ss.serialize_field("inclusion_filename", &self.with(filename))?;
      ss.end()
} 
SyntaxVariant::InclusionDirective (InclusionDirectiveChildren{expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "inclusion_directive")?;
      ss.serialize_field("inclusion_expression", &self.with(expression))?;
ss.serialize_field("inclusion_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::CompoundStatement (CompoundStatementChildren{left_brace,statements,right_brace} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "compound_statement")?;
      ss.serialize_field("compound_left_brace", &self.with(left_brace))?;
ss.serialize_field("compound_statements", &self.with(statements))?;
ss.serialize_field("compound_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ExpressionStatement (ExpressionStatementChildren{expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "expression_statement")?;
      ss.serialize_field("expression_statement_expression", &self.with(expression))?;
ss.serialize_field("expression_statement_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::MarkupSection (MarkupSectionChildren{hashbang,suffix} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "markup_section")?;
      ss.serialize_field("markup_hashbang", &self.with(hashbang))?;
ss.serialize_field("markup_suffix", &self.with(suffix))?;
      ss.end()
} 
SyntaxVariant::MarkupSuffix (MarkupSuffixChildren{less_than_question,name} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "markup_suffix")?;
      ss.serialize_field("markup_suffix_less_than_question", &self.with(less_than_question))?;
ss.serialize_field("markup_suffix_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::UnsetStatement (UnsetStatementChildren{keyword,left_paren,variables,right_paren,semicolon} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "unset_statement")?;
      ss.serialize_field("unset_keyword", &self.with(keyword))?;
ss.serialize_field("unset_left_paren", &self.with(left_paren))?;
ss.serialize_field("unset_variables", &self.with(variables))?;
ss.serialize_field("unset_right_paren", &self.with(right_paren))?;
ss.serialize_field("unset_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::DeclareLocalStatement (DeclareLocalStatementChildren{keyword,variable,colon,type_,initializer,semicolon} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "declare_local_statement")?;
      ss.serialize_field("declare_local_keyword", &self.with(keyword))?;
ss.serialize_field("declare_local_variable", &self.with(variable))?;
ss.serialize_field("declare_local_colon", &self.with(colon))?;
ss.serialize_field("declare_local_type", &self.with(type_))?;
ss.serialize_field("declare_local_initializer", &self.with(initializer))?;
ss.serialize_field("declare_local_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::UsingStatementBlockScoped (UsingStatementBlockScopedChildren{await_keyword,using_keyword,left_paren,expressions,right_paren,body} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "using_statement_block_scoped")?;
      ss.serialize_field("using_block_await_keyword", &self.with(await_keyword))?;
ss.serialize_field("using_block_using_keyword", &self.with(using_keyword))?;
ss.serialize_field("using_block_left_paren", &self.with(left_paren))?;
ss.serialize_field("using_block_expressions", &self.with(expressions))?;
ss.serialize_field("using_block_right_paren", &self.with(right_paren))?;
ss.serialize_field("using_block_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::UsingStatementFunctionScoped (UsingStatementFunctionScopedChildren{await_keyword,using_keyword,expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "using_statement_function_scoped")?;
      ss.serialize_field("using_function_await_keyword", &self.with(await_keyword))?;
ss.serialize_field("using_function_using_keyword", &self.with(using_keyword))?;
ss.serialize_field("using_function_expression", &self.with(expression))?;
ss.serialize_field("using_function_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::WhileStatement (WhileStatementChildren{keyword,left_paren,condition,right_paren,body} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "while_statement")?;
      ss.serialize_field("while_keyword", &self.with(keyword))?;
ss.serialize_field("while_left_paren", &self.with(left_paren))?;
ss.serialize_field("while_condition", &self.with(condition))?;
ss.serialize_field("while_right_paren", &self.with(right_paren))?;
ss.serialize_field("while_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::IfStatement (IfStatementChildren{keyword,left_paren,condition,right_paren,statement,else_clause} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "if_statement")?;
      ss.serialize_field("if_keyword", &self.with(keyword))?;
ss.serialize_field("if_left_paren", &self.with(left_paren))?;
ss.serialize_field("if_condition", &self.with(condition))?;
ss.serialize_field("if_right_paren", &self.with(right_paren))?;
ss.serialize_field("if_statement", &self.with(statement))?;
ss.serialize_field("if_else_clause", &self.with(else_clause))?;
      ss.end()
} 
SyntaxVariant::ElseClause (ElseClauseChildren{keyword,statement} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "else_clause")?;
      ss.serialize_field("else_keyword", &self.with(keyword))?;
ss.serialize_field("else_statement", &self.with(statement))?;
      ss.end()
} 
SyntaxVariant::TryStatement (TryStatementChildren{keyword,compound_statement,catch_clauses,finally_clause} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "try_statement")?;
      ss.serialize_field("try_keyword", &self.with(keyword))?;
ss.serialize_field("try_compound_statement", &self.with(compound_statement))?;
ss.serialize_field("try_catch_clauses", &self.with(catch_clauses))?;
ss.serialize_field("try_finally_clause", &self.with(finally_clause))?;
      ss.end()
} 
SyntaxVariant::CatchClause (CatchClauseChildren{keyword,left_paren,type_,variable,right_paren,body} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "catch_clause")?;
      ss.serialize_field("catch_keyword", &self.with(keyword))?;
ss.serialize_field("catch_left_paren", &self.with(left_paren))?;
ss.serialize_field("catch_type", &self.with(type_))?;
ss.serialize_field("catch_variable", &self.with(variable))?;
ss.serialize_field("catch_right_paren", &self.with(right_paren))?;
ss.serialize_field("catch_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::FinallyClause (FinallyClauseChildren{keyword,body} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "finally_clause")?;
      ss.serialize_field("finally_keyword", &self.with(keyword))?;
ss.serialize_field("finally_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::DoStatement (DoStatementChildren{keyword,body,while_keyword,left_paren,condition,right_paren,semicolon} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "do_statement")?;
      ss.serialize_field("do_keyword", &self.with(keyword))?;
ss.serialize_field("do_body", &self.with(body))?;
ss.serialize_field("do_while_keyword", &self.with(while_keyword))?;
ss.serialize_field("do_left_paren", &self.with(left_paren))?;
ss.serialize_field("do_condition", &self.with(condition))?;
ss.serialize_field("do_right_paren", &self.with(right_paren))?;
ss.serialize_field("do_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ForStatement (ForStatementChildren{keyword,left_paren,initializer,first_semicolon,control,second_semicolon,end_of_loop,right_paren,body} ) => {
      let mut ss = s.serialize_struct("", 10)?;
      ss.serialize_field("kind", "for_statement")?;
      ss.serialize_field("for_keyword", &self.with(keyword))?;
ss.serialize_field("for_left_paren", &self.with(left_paren))?;
ss.serialize_field("for_initializer", &self.with(initializer))?;
ss.serialize_field("for_first_semicolon", &self.with(first_semicolon))?;
ss.serialize_field("for_control", &self.with(control))?;
ss.serialize_field("for_second_semicolon", &self.with(second_semicolon))?;
ss.serialize_field("for_end_of_loop", &self.with(end_of_loop))?;
ss.serialize_field("for_right_paren", &self.with(right_paren))?;
ss.serialize_field("for_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::ForeachStatement (ForeachStatementChildren{keyword,left_paren,collection,await_keyword,as_,key,arrow,value,right_paren,body} ) => {
      let mut ss = s.serialize_struct("", 11)?;
      ss.serialize_field("kind", "foreach_statement")?;
      ss.serialize_field("foreach_keyword", &self.with(keyword))?;
ss.serialize_field("foreach_left_paren", &self.with(left_paren))?;
ss.serialize_field("foreach_collection", &self.with(collection))?;
ss.serialize_field("foreach_await_keyword", &self.with(await_keyword))?;
ss.serialize_field("foreach_as", &self.with(as_))?;
ss.serialize_field("foreach_key", &self.with(key))?;
ss.serialize_field("foreach_arrow", &self.with(arrow))?;
ss.serialize_field("foreach_value", &self.with(value))?;
ss.serialize_field("foreach_right_paren", &self.with(right_paren))?;
ss.serialize_field("foreach_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::SwitchStatement (SwitchStatementChildren{keyword,left_paren,expression,right_paren,left_brace,sections,right_brace} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "switch_statement")?;
      ss.serialize_field("switch_keyword", &self.with(keyword))?;
ss.serialize_field("switch_left_paren", &self.with(left_paren))?;
ss.serialize_field("switch_expression", &self.with(expression))?;
ss.serialize_field("switch_right_paren", &self.with(right_paren))?;
ss.serialize_field("switch_left_brace", &self.with(left_brace))?;
ss.serialize_field("switch_sections", &self.with(sections))?;
ss.serialize_field("switch_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::SwitchSection (SwitchSectionChildren{labels,statements,fallthrough} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "switch_section")?;
      ss.serialize_field("switch_section_labels", &self.with(labels))?;
ss.serialize_field("switch_section_statements", &self.with(statements))?;
ss.serialize_field("switch_section_fallthrough", &self.with(fallthrough))?;
      ss.end()
} 
SyntaxVariant::SwitchFallthrough (SwitchFallthroughChildren{keyword,semicolon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "switch_fallthrough")?;
      ss.serialize_field("fallthrough_keyword", &self.with(keyword))?;
ss.serialize_field("fallthrough_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::CaseLabel (CaseLabelChildren{keyword,expression,colon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "case_label")?;
      ss.serialize_field("case_keyword", &self.with(keyword))?;
ss.serialize_field("case_expression", &self.with(expression))?;
ss.serialize_field("case_colon", &self.with(colon))?;
      ss.end()
} 
SyntaxVariant::DefaultLabel (DefaultLabelChildren{keyword,colon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "default_label")?;
      ss.serialize_field("default_keyword", &self.with(keyword))?;
ss.serialize_field("default_colon", &self.with(colon))?;
      ss.end()
} 
SyntaxVariant::MatchStatement (MatchStatementChildren{keyword,left_paren,expression,right_paren,left_brace,arms,right_brace} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "match_statement")?;
      ss.serialize_field("match_statement_keyword", &self.with(keyword))?;
ss.serialize_field("match_statement_left_paren", &self.with(left_paren))?;
ss.serialize_field("match_statement_expression", &self.with(expression))?;
ss.serialize_field("match_statement_right_paren", &self.with(right_paren))?;
ss.serialize_field("match_statement_left_brace", &self.with(left_brace))?;
ss.serialize_field("match_statement_arms", &self.with(arms))?;
ss.serialize_field("match_statement_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::MatchStatementArm (MatchStatementArmChildren{pattern,arrow,body} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "match_statement_arm")?;
      ss.serialize_field("match_statement_arm_pattern", &self.with(pattern))?;
ss.serialize_field("match_statement_arm_arrow", &self.with(arrow))?;
ss.serialize_field("match_statement_arm_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::ReturnStatement (ReturnStatementChildren{keyword,expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "return_statement")?;
      ss.serialize_field("return_keyword", &self.with(keyword))?;
ss.serialize_field("return_expression", &self.with(expression))?;
ss.serialize_field("return_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::YieldBreakStatement (YieldBreakStatementChildren{keyword,break_,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "yield_break_statement")?;
      ss.serialize_field("yield_break_keyword", &self.with(keyword))?;
ss.serialize_field("yield_break_break", &self.with(break_))?;
ss.serialize_field("yield_break_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ThrowStatement (ThrowStatementChildren{keyword,expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "throw_statement")?;
      ss.serialize_field("throw_keyword", &self.with(keyword))?;
ss.serialize_field("throw_expression", &self.with(expression))?;
ss.serialize_field("throw_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::BreakStatement (BreakStatementChildren{keyword,semicolon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "break_statement")?;
      ss.serialize_field("break_keyword", &self.with(keyword))?;
ss.serialize_field("break_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ContinueStatement (ContinueStatementChildren{keyword,semicolon} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "continue_statement")?;
      ss.serialize_field("continue_keyword", &self.with(keyword))?;
ss.serialize_field("continue_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::EchoStatement (EchoStatementChildren{keyword,expressions,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "echo_statement")?;
      ss.serialize_field("echo_keyword", &self.with(keyword))?;
ss.serialize_field("echo_expressions", &self.with(expressions))?;
ss.serialize_field("echo_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::ConcurrentStatement (ConcurrentStatementChildren{keyword,statement} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "concurrent_statement")?;
      ss.serialize_field("concurrent_keyword", &self.with(keyword))?;
ss.serialize_field("concurrent_statement", &self.with(statement))?;
      ss.end()
} 
SyntaxVariant::SimpleInitializer (SimpleInitializerChildren{equal,value} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "simple_initializer")?;
      ss.serialize_field("simple_initializer_equal", &self.with(equal))?;
ss.serialize_field("simple_initializer_value", &self.with(value))?;
      ss.end()
} 
SyntaxVariant::AnonymousClass (AnonymousClassChildren{class_keyword,left_paren,argument_list,right_paren,extends_keyword,extends_list,implements_keyword,implements_list,body} ) => {
      let mut ss = s.serialize_struct("", 10)?;
      ss.serialize_field("kind", "anonymous_class")?;
      ss.serialize_field("anonymous_class_class_keyword", &self.with(class_keyword))?;
ss.serialize_field("anonymous_class_left_paren", &self.with(left_paren))?;
ss.serialize_field("anonymous_class_argument_list", &self.with(argument_list))?;
ss.serialize_field("anonymous_class_right_paren", &self.with(right_paren))?;
ss.serialize_field("anonymous_class_extends_keyword", &self.with(extends_keyword))?;
ss.serialize_field("anonymous_class_extends_list", &self.with(extends_list))?;
ss.serialize_field("anonymous_class_implements_keyword", &self.with(implements_keyword))?;
ss.serialize_field("anonymous_class_implements_list", &self.with(implements_list))?;
ss.serialize_field("anonymous_class_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::AnonymousFunction (AnonymousFunctionChildren{attribute_spec,async_keyword,function_keyword,left_paren,parameters,right_paren,ctx_list,colon,readonly_return,type_,use_,body} ) => {
      let mut ss = s.serialize_struct("", 13)?;
      ss.serialize_field("kind", "anonymous_function")?;
      ss.serialize_field("anonymous_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("anonymous_async_keyword", &self.with(async_keyword))?;
ss.serialize_field("anonymous_function_keyword", &self.with(function_keyword))?;
ss.serialize_field("anonymous_left_paren", &self.with(left_paren))?;
ss.serialize_field("anonymous_parameters", &self.with(parameters))?;
ss.serialize_field("anonymous_right_paren", &self.with(right_paren))?;
ss.serialize_field("anonymous_ctx_list", &self.with(ctx_list))?;
ss.serialize_field("anonymous_colon", &self.with(colon))?;
ss.serialize_field("anonymous_readonly_return", &self.with(readonly_return))?;
ss.serialize_field("anonymous_type", &self.with(type_))?;
ss.serialize_field("anonymous_use", &self.with(use_))?;
ss.serialize_field("anonymous_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::AnonymousFunctionUseClause (AnonymousFunctionUseClauseChildren{keyword,left_paren,variables,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "anonymous_function_use_clause")?;
      ss.serialize_field("anonymous_use_keyword", &self.with(keyword))?;
ss.serialize_field("anonymous_use_left_paren", &self.with(left_paren))?;
ss.serialize_field("anonymous_use_variables", &self.with(variables))?;
ss.serialize_field("anonymous_use_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::VariablePattern (VariablePatternChildren{variable} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "variable_pattern")?;
      ss.serialize_field("variable_pattern_variable", &self.with(variable))?;
      ss.end()
} 
SyntaxVariant::ConstructorPattern (ConstructorPatternChildren{constructor,left_paren,members,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "constructor_pattern")?;
      ss.serialize_field("constructor_pattern_constructor", &self.with(constructor))?;
ss.serialize_field("constructor_pattern_left_paren", &self.with(left_paren))?;
ss.serialize_field("constructor_pattern_members", &self.with(members))?;
ss.serialize_field("constructor_pattern_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::RefinementPattern (RefinementPatternChildren{variable,colon,specifier} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "refinement_pattern")?;
      ss.serialize_field("refinement_pattern_variable", &self.with(variable))?;
ss.serialize_field("refinement_pattern_colon", &self.with(colon))?;
ss.serialize_field("refinement_pattern_specifier", &self.with(specifier))?;
      ss.end()
} 
SyntaxVariant::LambdaExpression (LambdaExpressionChildren{attribute_spec,async_,signature,arrow,body} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "lambda_expression")?;
      ss.serialize_field("lambda_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("lambda_async", &self.with(async_))?;
ss.serialize_field("lambda_signature", &self.with(signature))?;
ss.serialize_field("lambda_arrow", &self.with(arrow))?;
ss.serialize_field("lambda_body", &self.with(body))?;
      ss.end()
} 
SyntaxVariant::LambdaSignature (LambdaSignatureChildren{left_paren,parameters,right_paren,contexts,colon,readonly_return,type_} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "lambda_signature")?;
      ss.serialize_field("lambda_left_paren", &self.with(left_paren))?;
ss.serialize_field("lambda_parameters", &self.with(parameters))?;
ss.serialize_field("lambda_right_paren", &self.with(right_paren))?;
ss.serialize_field("lambda_contexts", &self.with(contexts))?;
ss.serialize_field("lambda_colon", &self.with(colon))?;
ss.serialize_field("lambda_readonly_return", &self.with(readonly_return))?;
ss.serialize_field("lambda_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::CastExpression (CastExpressionChildren{left_paren,type_,right_paren,operand} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "cast_expression")?;
      ss.serialize_field("cast_left_paren", &self.with(left_paren))?;
ss.serialize_field("cast_type", &self.with(type_))?;
ss.serialize_field("cast_right_paren", &self.with(right_paren))?;
ss.serialize_field("cast_operand", &self.with(operand))?;
      ss.end()
} 
SyntaxVariant::ScopeResolutionExpression (ScopeResolutionExpressionChildren{qualifier,operator,name} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "scope_resolution_expression")?;
      ss.serialize_field("scope_resolution_qualifier", &self.with(qualifier))?;
ss.serialize_field("scope_resolution_operator", &self.with(operator))?;
ss.serialize_field("scope_resolution_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::MemberSelectionExpression (MemberSelectionExpressionChildren{object,operator,name} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "member_selection_expression")?;
      ss.serialize_field("member_object", &self.with(object))?;
ss.serialize_field("member_operator", &self.with(operator))?;
ss.serialize_field("member_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::SafeMemberSelectionExpression (SafeMemberSelectionExpressionChildren{object,operator,name} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "safe_member_selection_expression")?;
      ss.serialize_field("safe_member_object", &self.with(object))?;
ss.serialize_field("safe_member_operator", &self.with(operator))?;
ss.serialize_field("safe_member_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::EmbeddedMemberSelectionExpression (EmbeddedMemberSelectionExpressionChildren{object,operator,name} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "embedded_member_selection_expression")?;
      ss.serialize_field("embedded_member_object", &self.with(object))?;
ss.serialize_field("embedded_member_operator", &self.with(operator))?;
ss.serialize_field("embedded_member_name", &self.with(name))?;
      ss.end()
} 
SyntaxVariant::YieldExpression (YieldExpressionChildren{keyword,operand} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "yield_expression")?;
      ss.serialize_field("yield_keyword", &self.with(keyword))?;
ss.serialize_field("yield_operand", &self.with(operand))?;
      ss.end()
} 
SyntaxVariant::PrefixUnaryExpression (PrefixUnaryExpressionChildren{operator,operand} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "prefix_unary_expression")?;
      ss.serialize_field("prefix_unary_operator", &self.with(operator))?;
ss.serialize_field("prefix_unary_operand", &self.with(operand))?;
      ss.end()
} 
SyntaxVariant::PostfixUnaryExpression (PostfixUnaryExpressionChildren{operand,operator} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "postfix_unary_expression")?;
      ss.serialize_field("postfix_unary_operand", &self.with(operand))?;
ss.serialize_field("postfix_unary_operator", &self.with(operator))?;
      ss.end()
} 
SyntaxVariant::BinaryExpression (BinaryExpressionChildren{left_operand,operator,right_operand} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "binary_expression")?;
      ss.serialize_field("binary_left_operand", &self.with(left_operand))?;
ss.serialize_field("binary_operator", &self.with(operator))?;
ss.serialize_field("binary_right_operand", &self.with(right_operand))?;
      ss.end()
} 
SyntaxVariant::IsExpression (IsExpressionChildren{left_operand,operator,right_operand} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "is_expression")?;
      ss.serialize_field("is_left_operand", &self.with(left_operand))?;
ss.serialize_field("is_operator", &self.with(operator))?;
ss.serialize_field("is_right_operand", &self.with(right_operand))?;
      ss.end()
} 
SyntaxVariant::AsExpression (AsExpressionChildren{left_operand,operator,right_operand} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "as_expression")?;
      ss.serialize_field("as_left_operand", &self.with(left_operand))?;
ss.serialize_field("as_operator", &self.with(operator))?;
ss.serialize_field("as_right_operand", &self.with(right_operand))?;
      ss.end()
} 
SyntaxVariant::NullableAsExpression (NullableAsExpressionChildren{left_operand,operator,right_operand} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "nullable_as_expression")?;
      ss.serialize_field("nullable_as_left_operand", &self.with(left_operand))?;
ss.serialize_field("nullable_as_operator", &self.with(operator))?;
ss.serialize_field("nullable_as_right_operand", &self.with(right_operand))?;
      ss.end()
} 
SyntaxVariant::UpcastExpression (UpcastExpressionChildren{left_operand,operator,right_operand} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "upcast_expression")?;
      ss.serialize_field("upcast_left_operand", &self.with(left_operand))?;
ss.serialize_field("upcast_operator", &self.with(operator))?;
ss.serialize_field("upcast_right_operand", &self.with(right_operand))?;
      ss.end()
} 
SyntaxVariant::ConditionalExpression (ConditionalExpressionChildren{test,question,consequence,colon,alternative} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "conditional_expression")?;
      ss.serialize_field("conditional_test", &self.with(test))?;
ss.serialize_field("conditional_question", &self.with(question))?;
ss.serialize_field("conditional_consequence", &self.with(consequence))?;
ss.serialize_field("conditional_colon", &self.with(colon))?;
ss.serialize_field("conditional_alternative", &self.with(alternative))?;
      ss.end()
} 
SyntaxVariant::EvalExpression (EvalExpressionChildren{keyword,left_paren,argument,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "eval_expression")?;
      ss.serialize_field("eval_keyword", &self.with(keyword))?;
ss.serialize_field("eval_left_paren", &self.with(left_paren))?;
ss.serialize_field("eval_argument", &self.with(argument))?;
ss.serialize_field("eval_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::IssetExpression (IssetExpressionChildren{keyword,left_paren,argument_list,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "isset_expression")?;
      ss.serialize_field("isset_keyword", &self.with(keyword))?;
ss.serialize_field("isset_left_paren", &self.with(left_paren))?;
ss.serialize_field("isset_argument_list", &self.with(argument_list))?;
ss.serialize_field("isset_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::NameofExpression (NameofExpressionChildren{keyword,target} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "nameof_expression")?;
      ss.serialize_field("nameof_keyword", &self.with(keyword))?;
ss.serialize_field("nameof_target", &self.with(target))?;
      ss.end()
} 
SyntaxVariant::FunctionCallExpression (FunctionCallExpressionChildren{receiver,type_args,left_paren,argument_list,right_paren} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "function_call_expression")?;
      ss.serialize_field("function_call_receiver", &self.with(receiver))?;
ss.serialize_field("function_call_type_args", &self.with(type_args))?;
ss.serialize_field("function_call_left_paren", &self.with(left_paren))?;
ss.serialize_field("function_call_argument_list", &self.with(argument_list))?;
ss.serialize_field("function_call_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::FunctionPointerExpression (FunctionPointerExpressionChildren{receiver,type_args} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "function_pointer_expression")?;
      ss.serialize_field("function_pointer_receiver", &self.with(receiver))?;
ss.serialize_field("function_pointer_type_args", &self.with(type_args))?;
      ss.end()
} 
SyntaxVariant::ParenthesizedExpression (ParenthesizedExpressionChildren{left_paren,expression,right_paren} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "parenthesized_expression")?;
      ss.serialize_field("parenthesized_expression_left_paren", &self.with(left_paren))?;
ss.serialize_field("parenthesized_expression_expression", &self.with(expression))?;
ss.serialize_field("parenthesized_expression_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::BracedExpression (BracedExpressionChildren{left_brace,expression,right_brace} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "braced_expression")?;
      ss.serialize_field("braced_expression_left_brace", &self.with(left_brace))?;
ss.serialize_field("braced_expression_expression", &self.with(expression))?;
ss.serialize_field("braced_expression_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ETSpliceExpression (ETSpliceExpressionChildren{dollar,left_brace,expression,right_brace} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "et_splice_expression")?;
      ss.serialize_field("et_splice_expression_dollar", &self.with(dollar))?;
ss.serialize_field("et_splice_expression_left_brace", &self.with(left_brace))?;
ss.serialize_field("et_splice_expression_expression", &self.with(expression))?;
ss.serialize_field("et_splice_expression_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::EmbeddedBracedExpression (EmbeddedBracedExpressionChildren{left_brace,expression,right_brace} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "embedded_braced_expression")?;
      ss.serialize_field("embedded_braced_expression_left_brace", &self.with(left_brace))?;
ss.serialize_field("embedded_braced_expression_expression", &self.with(expression))?;
ss.serialize_field("embedded_braced_expression_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ListExpression (ListExpressionChildren{keyword,left_paren,members,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "list_expression")?;
      ss.serialize_field("list_keyword", &self.with(keyword))?;
ss.serialize_field("list_left_paren", &self.with(left_paren))?;
ss.serialize_field("list_members", &self.with(members))?;
ss.serialize_field("list_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::CollectionLiteralExpression (CollectionLiteralExpressionChildren{name,left_brace,initializers,right_brace} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "collection_literal_expression")?;
      ss.serialize_field("collection_literal_name", &self.with(name))?;
ss.serialize_field("collection_literal_left_brace", &self.with(left_brace))?;
ss.serialize_field("collection_literal_initializers", &self.with(initializers))?;
ss.serialize_field("collection_literal_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ObjectCreationExpression (ObjectCreationExpressionChildren{new_keyword,object} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "object_creation_expression")?;
      ss.serialize_field("object_creation_new_keyword", &self.with(new_keyword))?;
ss.serialize_field("object_creation_object", &self.with(object))?;
      ss.end()
} 
SyntaxVariant::ConstructorCall (ConstructorCallChildren{type_,left_paren,argument_list,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "constructor_call")?;
      ss.serialize_field("constructor_call_type", &self.with(type_))?;
ss.serialize_field("constructor_call_left_paren", &self.with(left_paren))?;
ss.serialize_field("constructor_call_argument_list", &self.with(argument_list))?;
ss.serialize_field("constructor_call_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::DarrayIntrinsicExpression (DarrayIntrinsicExpressionChildren{keyword,explicit_type,left_bracket,members,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "darray_intrinsic_expression")?;
      ss.serialize_field("darray_intrinsic_keyword", &self.with(keyword))?;
ss.serialize_field("darray_intrinsic_explicit_type", &self.with(explicit_type))?;
ss.serialize_field("darray_intrinsic_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("darray_intrinsic_members", &self.with(members))?;
ss.serialize_field("darray_intrinsic_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::DictionaryIntrinsicExpression (DictionaryIntrinsicExpressionChildren{keyword,explicit_type,left_bracket,members,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "dictionary_intrinsic_expression")?;
      ss.serialize_field("dictionary_intrinsic_keyword", &self.with(keyword))?;
ss.serialize_field("dictionary_intrinsic_explicit_type", &self.with(explicit_type))?;
ss.serialize_field("dictionary_intrinsic_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("dictionary_intrinsic_members", &self.with(members))?;
ss.serialize_field("dictionary_intrinsic_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::KeysetIntrinsicExpression (KeysetIntrinsicExpressionChildren{keyword,explicit_type,left_bracket,members,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "keyset_intrinsic_expression")?;
      ss.serialize_field("keyset_intrinsic_keyword", &self.with(keyword))?;
ss.serialize_field("keyset_intrinsic_explicit_type", &self.with(explicit_type))?;
ss.serialize_field("keyset_intrinsic_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("keyset_intrinsic_members", &self.with(members))?;
ss.serialize_field("keyset_intrinsic_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::VarrayIntrinsicExpression (VarrayIntrinsicExpressionChildren{keyword,explicit_type,left_bracket,members,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "varray_intrinsic_expression")?;
      ss.serialize_field("varray_intrinsic_keyword", &self.with(keyword))?;
ss.serialize_field("varray_intrinsic_explicit_type", &self.with(explicit_type))?;
ss.serialize_field("varray_intrinsic_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("varray_intrinsic_members", &self.with(members))?;
ss.serialize_field("varray_intrinsic_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::VectorIntrinsicExpression (VectorIntrinsicExpressionChildren{keyword,explicit_type,left_bracket,members,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "vector_intrinsic_expression")?;
      ss.serialize_field("vector_intrinsic_keyword", &self.with(keyword))?;
ss.serialize_field("vector_intrinsic_explicit_type", &self.with(explicit_type))?;
ss.serialize_field("vector_intrinsic_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("vector_intrinsic_members", &self.with(members))?;
ss.serialize_field("vector_intrinsic_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::ElementInitializer (ElementInitializerChildren{key,arrow,value} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "element_initializer")?;
      ss.serialize_field("element_key", &self.with(key))?;
ss.serialize_field("element_arrow", &self.with(arrow))?;
ss.serialize_field("element_value", &self.with(value))?;
      ss.end()
} 
SyntaxVariant::SubscriptExpression (SubscriptExpressionChildren{receiver,left_bracket,index,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "subscript_expression")?;
      ss.serialize_field("subscript_receiver", &self.with(receiver))?;
ss.serialize_field("subscript_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("subscript_index", &self.with(index))?;
ss.serialize_field("subscript_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::EmbeddedSubscriptExpression (EmbeddedSubscriptExpressionChildren{receiver,left_bracket,index,right_bracket} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "embedded_subscript_expression")?;
      ss.serialize_field("embedded_subscript_receiver", &self.with(receiver))?;
ss.serialize_field("embedded_subscript_left_bracket", &self.with(left_bracket))?;
ss.serialize_field("embedded_subscript_index", &self.with(index))?;
ss.serialize_field("embedded_subscript_right_bracket", &self.with(right_bracket))?;
      ss.end()
} 
SyntaxVariant::AwaitableCreationExpression (AwaitableCreationExpressionChildren{attribute_spec,async_,compound_statement} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "awaitable_creation_expression")?;
      ss.serialize_field("awaitable_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("awaitable_async", &self.with(async_))?;
ss.serialize_field("awaitable_compound_statement", &self.with(compound_statement))?;
      ss.end()
} 
SyntaxVariant::XHPChildrenDeclaration (XHPChildrenDeclarationChildren{keyword,expression,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_children_declaration")?;
      ss.serialize_field("xhp_children_keyword", &self.with(keyword))?;
ss.serialize_field("xhp_children_expression", &self.with(expression))?;
ss.serialize_field("xhp_children_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::XHPChildrenParenthesizedList (XHPChildrenParenthesizedListChildren{left_paren,xhp_children,right_paren} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_children_parenthesized_list")?;
      ss.serialize_field("xhp_children_list_left_paren", &self.with(left_paren))?;
ss.serialize_field("xhp_children_list_xhp_children", &self.with(xhp_children))?;
ss.serialize_field("xhp_children_list_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::XHPCategoryDeclaration (XHPCategoryDeclarationChildren{keyword,categories,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_category_declaration")?;
      ss.serialize_field("xhp_category_keyword", &self.with(keyword))?;
ss.serialize_field("xhp_category_categories", &self.with(categories))?;
ss.serialize_field("xhp_category_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::XHPEnumType (XHPEnumTypeChildren{like,keyword,left_brace,values,right_brace} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "xhp_enum_type")?;
      ss.serialize_field("xhp_enum_like", &self.with(like))?;
ss.serialize_field("xhp_enum_keyword", &self.with(keyword))?;
ss.serialize_field("xhp_enum_left_brace", &self.with(left_brace))?;
ss.serialize_field("xhp_enum_values", &self.with(values))?;
ss.serialize_field("xhp_enum_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::XHPLateinit (XHPLateinitChildren{at,keyword} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "xhp_lateinit")?;
      ss.serialize_field("xhp_lateinit_at", &self.with(at))?;
ss.serialize_field("xhp_lateinit_keyword", &self.with(keyword))?;
      ss.end()
} 
SyntaxVariant::XHPRequired (XHPRequiredChildren{at,keyword} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "xhp_required")?;
      ss.serialize_field("xhp_required_at", &self.with(at))?;
ss.serialize_field("xhp_required_keyword", &self.with(keyword))?;
      ss.end()
} 
SyntaxVariant::XHPClassAttributeDeclaration (XHPClassAttributeDeclarationChildren{keyword,attributes,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_class_attribute_declaration")?;
      ss.serialize_field("xhp_attribute_keyword", &self.with(keyword))?;
ss.serialize_field("xhp_attribute_attributes", &self.with(attributes))?;
ss.serialize_field("xhp_attribute_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::XHPClassAttribute (XHPClassAttributeChildren{type_,name,initializer,required} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "xhp_class_attribute")?;
      ss.serialize_field("xhp_attribute_decl_type", &self.with(type_))?;
ss.serialize_field("xhp_attribute_decl_name", &self.with(name))?;
ss.serialize_field("xhp_attribute_decl_initializer", &self.with(initializer))?;
ss.serialize_field("xhp_attribute_decl_required", &self.with(required))?;
      ss.end()
} 
SyntaxVariant::XHPSimpleClassAttribute (XHPSimpleClassAttributeChildren{type_} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "xhp_simple_class_attribute")?;
      ss.serialize_field("xhp_simple_class_attribute_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::XHPSimpleAttribute (XHPSimpleAttributeChildren{name,equal,expression} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_simple_attribute")?;
      ss.serialize_field("xhp_simple_attribute_name", &self.with(name))?;
ss.serialize_field("xhp_simple_attribute_equal", &self.with(equal))?;
ss.serialize_field("xhp_simple_attribute_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::XHPSpreadAttribute (XHPSpreadAttributeChildren{left_brace,spread_operator,expression,right_brace} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "xhp_spread_attribute")?;
      ss.serialize_field("xhp_spread_attribute_left_brace", &self.with(left_brace))?;
ss.serialize_field("xhp_spread_attribute_spread_operator", &self.with(spread_operator))?;
ss.serialize_field("xhp_spread_attribute_expression", &self.with(expression))?;
ss.serialize_field("xhp_spread_attribute_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::XHPOpen (XHPOpenChildren{left_angle,name,attributes,right_angle} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "xhp_open")?;
      ss.serialize_field("xhp_open_left_angle", &self.with(left_angle))?;
ss.serialize_field("xhp_open_name", &self.with(name))?;
ss.serialize_field("xhp_open_attributes", &self.with(attributes))?;
ss.serialize_field("xhp_open_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::XHPExpression (XHPExpressionChildren{open,body,close} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_expression")?;
      ss.serialize_field("xhp_open", &self.with(open))?;
ss.serialize_field("xhp_body", &self.with(body))?;
ss.serialize_field("xhp_close", &self.with(close))?;
      ss.end()
} 
SyntaxVariant::XHPClose (XHPCloseChildren{left_angle,name,right_angle} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "xhp_close")?;
      ss.serialize_field("xhp_close_left_angle", &self.with(left_angle))?;
ss.serialize_field("xhp_close_name", &self.with(name))?;
ss.serialize_field("xhp_close_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::TypeConstant (TypeConstantChildren{left_type,separator,right_type} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "type_constant")?;
      ss.serialize_field("type_constant_left_type", &self.with(left_type))?;
ss.serialize_field("type_constant_separator", &self.with(separator))?;
ss.serialize_field("type_constant_right_type", &self.with(right_type))?;
      ss.end()
} 
SyntaxVariant::VectorTypeSpecifier (VectorTypeSpecifierChildren{keyword,left_angle,type_,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "vector_type_specifier")?;
      ss.serialize_field("vector_type_keyword", &self.with(keyword))?;
ss.serialize_field("vector_type_left_angle", &self.with(left_angle))?;
ss.serialize_field("vector_type_type", &self.with(type_))?;
ss.serialize_field("vector_type_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("vector_type_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::KeysetTypeSpecifier (KeysetTypeSpecifierChildren{keyword,left_angle,type_,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "keyset_type_specifier")?;
      ss.serialize_field("keyset_type_keyword", &self.with(keyword))?;
ss.serialize_field("keyset_type_left_angle", &self.with(left_angle))?;
ss.serialize_field("keyset_type_type", &self.with(type_))?;
ss.serialize_field("keyset_type_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("keyset_type_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::TupleTypeExplicitSpecifier (TupleTypeExplicitSpecifierChildren{keyword,left_angle,types,right_angle} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "tuple_type_explicit_specifier")?;
      ss.serialize_field("tuple_type_keyword", &self.with(keyword))?;
ss.serialize_field("tuple_type_left_angle", &self.with(left_angle))?;
ss.serialize_field("tuple_type_types", &self.with(types))?;
ss.serialize_field("tuple_type_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::VarrayTypeSpecifier (VarrayTypeSpecifierChildren{keyword,left_angle,type_,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "varray_type_specifier")?;
      ss.serialize_field("varray_keyword", &self.with(keyword))?;
ss.serialize_field("varray_left_angle", &self.with(left_angle))?;
ss.serialize_field("varray_type", &self.with(type_))?;
ss.serialize_field("varray_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("varray_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::FunctionCtxTypeSpecifier (FunctionCtxTypeSpecifierChildren{keyword,variable} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "function_ctx_type_specifier")?;
      ss.serialize_field("function_ctx_type_keyword", &self.with(keyword))?;
ss.serialize_field("function_ctx_type_variable", &self.with(variable))?;
      ss.end()
} 
SyntaxVariant::TypeParameter (TypeParameterChildren{attribute_spec,reified,variance,name,param_params,constraints} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "type_parameter")?;
      ss.serialize_field("type_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("type_reified", &self.with(reified))?;
ss.serialize_field("type_variance", &self.with(variance))?;
ss.serialize_field("type_name", &self.with(name))?;
ss.serialize_field("type_param_params", &self.with(param_params))?;
ss.serialize_field("type_constraints", &self.with(constraints))?;
      ss.end()
} 
SyntaxVariant::TypeConstraint (TypeConstraintChildren{keyword,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "type_constraint")?;
      ss.serialize_field("constraint_keyword", &self.with(keyword))?;
ss.serialize_field("constraint_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::ContextConstraint (ContextConstraintChildren{keyword,ctx_list} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "context_constraint")?;
      ss.serialize_field("ctx_constraint_keyword", &self.with(keyword))?;
ss.serialize_field("ctx_constraint_ctx_list", &self.with(ctx_list))?;
      ss.end()
} 
SyntaxVariant::DarrayTypeSpecifier (DarrayTypeSpecifierChildren{keyword,left_angle,key,comma,value,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 8)?;
      ss.serialize_field("kind", "darray_type_specifier")?;
      ss.serialize_field("darray_keyword", &self.with(keyword))?;
ss.serialize_field("darray_left_angle", &self.with(left_angle))?;
ss.serialize_field("darray_key", &self.with(key))?;
ss.serialize_field("darray_comma", &self.with(comma))?;
ss.serialize_field("darray_value", &self.with(value))?;
ss.serialize_field("darray_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("darray_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::DictionaryTypeSpecifier (DictionaryTypeSpecifierChildren{keyword,left_angle,members,right_angle} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "dictionary_type_specifier")?;
      ss.serialize_field("dictionary_type_keyword", &self.with(keyword))?;
ss.serialize_field("dictionary_type_left_angle", &self.with(left_angle))?;
ss.serialize_field("dictionary_type_members", &self.with(members))?;
ss.serialize_field("dictionary_type_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::ClosureTypeSpecifier (ClosureTypeSpecifierChildren{outer_left_paren,readonly_keyword,function_keyword,inner_left_paren,parameter_list,inner_right_paren,contexts,colon,readonly_return,return_type,outer_right_paren} ) => {
      let mut ss = s.serialize_struct("", 12)?;
      ss.serialize_field("kind", "closure_type_specifier")?;
      ss.serialize_field("closure_outer_left_paren", &self.with(outer_left_paren))?;
ss.serialize_field("closure_readonly_keyword", &self.with(readonly_keyword))?;
ss.serialize_field("closure_function_keyword", &self.with(function_keyword))?;
ss.serialize_field("closure_inner_left_paren", &self.with(inner_left_paren))?;
ss.serialize_field("closure_parameter_list", &self.with(parameter_list))?;
ss.serialize_field("closure_inner_right_paren", &self.with(inner_right_paren))?;
ss.serialize_field("closure_contexts", &self.with(contexts))?;
ss.serialize_field("closure_colon", &self.with(colon))?;
ss.serialize_field("closure_readonly_return", &self.with(readonly_return))?;
ss.serialize_field("closure_return_type", &self.with(return_type))?;
ss.serialize_field("closure_outer_right_paren", &self.with(outer_right_paren))?;
      ss.end()
} 
SyntaxVariant::ClosureParameterTypeSpecifier (ClosureParameterTypeSpecifierChildren{optional,call_convention,readonly,type_} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "closure_parameter_type_specifier")?;
      ss.serialize_field("closure_parameter_optional", &self.with(optional))?;
ss.serialize_field("closure_parameter_call_convention", &self.with(call_convention))?;
ss.serialize_field("closure_parameter_readonly", &self.with(readonly))?;
ss.serialize_field("closure_parameter_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::TypeRefinement (TypeRefinementChildren{type_,keyword,left_brace,members,right_brace} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "type_refinement")?;
      ss.serialize_field("type_refinement_type", &self.with(type_))?;
ss.serialize_field("type_refinement_keyword", &self.with(keyword))?;
ss.serialize_field("type_refinement_left_brace", &self.with(left_brace))?;
ss.serialize_field("type_refinement_members", &self.with(members))?;
ss.serialize_field("type_refinement_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::TypeInRefinement (TypeInRefinementChildren{keyword,name,type_parameters,constraints,equal,type_} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "type_in_refinement")?;
      ss.serialize_field("type_in_refinement_keyword", &self.with(keyword))?;
ss.serialize_field("type_in_refinement_name", &self.with(name))?;
ss.serialize_field("type_in_refinement_type_parameters", &self.with(type_parameters))?;
ss.serialize_field("type_in_refinement_constraints", &self.with(constraints))?;
ss.serialize_field("type_in_refinement_equal", &self.with(equal))?;
ss.serialize_field("type_in_refinement_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::CtxInRefinement (CtxInRefinementChildren{keyword,name,type_parameters,constraints,equal,ctx_list} ) => {
      let mut ss = s.serialize_struct("", 7)?;
      ss.serialize_field("kind", "ctx_in_refinement")?;
      ss.serialize_field("ctx_in_refinement_keyword", &self.with(keyword))?;
ss.serialize_field("ctx_in_refinement_name", &self.with(name))?;
ss.serialize_field("ctx_in_refinement_type_parameters", &self.with(type_parameters))?;
ss.serialize_field("ctx_in_refinement_constraints", &self.with(constraints))?;
ss.serialize_field("ctx_in_refinement_equal", &self.with(equal))?;
ss.serialize_field("ctx_in_refinement_ctx_list", &self.with(ctx_list))?;
      ss.end()
} 
SyntaxVariant::ClassnameTypeSpecifier (ClassnameTypeSpecifierChildren{keyword,left_angle,type_,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "classname_type_specifier")?;
      ss.serialize_field("classname_keyword", &self.with(keyword))?;
ss.serialize_field("classname_left_angle", &self.with(left_angle))?;
ss.serialize_field("classname_type", &self.with(type_))?;
ss.serialize_field("classname_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("classname_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::ClassArgsTypeSpecifier (ClassArgsTypeSpecifierChildren{keyword,left_angle,type_,trailing_comma,right_angle} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "class_args_type_specifier")?;
      ss.serialize_field("class_args_keyword", &self.with(keyword))?;
ss.serialize_field("class_args_left_angle", &self.with(left_angle))?;
ss.serialize_field("class_args_type", &self.with(type_))?;
ss.serialize_field("class_args_trailing_comma", &self.with(trailing_comma))?;
ss.serialize_field("class_args_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::FieldSpecifier (FieldSpecifierChildren{question,name,arrow,type_} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "field_specifier")?;
      ss.serialize_field("field_question", &self.with(question))?;
ss.serialize_field("field_name", &self.with(name))?;
ss.serialize_field("field_arrow", &self.with(arrow))?;
ss.serialize_field("field_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::FieldInitializer (FieldInitializerChildren{name,arrow,value} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "field_initializer")?;
      ss.serialize_field("field_initializer_name", &self.with(name))?;
ss.serialize_field("field_initializer_arrow", &self.with(arrow))?;
ss.serialize_field("field_initializer_value", &self.with(value))?;
      ss.end()
} 
SyntaxVariant::ShapeTypeSpecifier (ShapeTypeSpecifierChildren{keyword,left_paren,fields,ellipsis,right_paren} ) => {
      let mut ss = s.serialize_struct("", 6)?;
      ss.serialize_field("kind", "shape_type_specifier")?;
      ss.serialize_field("shape_type_keyword", &self.with(keyword))?;
ss.serialize_field("shape_type_left_paren", &self.with(left_paren))?;
ss.serialize_field("shape_type_fields", &self.with(fields))?;
ss.serialize_field("shape_type_ellipsis", &self.with(ellipsis))?;
ss.serialize_field("shape_type_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::ShapeExpression (ShapeExpressionChildren{keyword,left_paren,fields,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "shape_expression")?;
      ss.serialize_field("shape_expression_keyword", &self.with(keyword))?;
ss.serialize_field("shape_expression_left_paren", &self.with(left_paren))?;
ss.serialize_field("shape_expression_fields", &self.with(fields))?;
ss.serialize_field("shape_expression_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::TupleExpression (TupleExpressionChildren{keyword,left_paren,items,right_paren} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "tuple_expression")?;
      ss.serialize_field("tuple_expression_keyword", &self.with(keyword))?;
ss.serialize_field("tuple_expression_left_paren", &self.with(left_paren))?;
ss.serialize_field("tuple_expression_items", &self.with(items))?;
ss.serialize_field("tuple_expression_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::GenericTypeSpecifier (GenericTypeSpecifierChildren{class_type,argument_list} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "generic_type_specifier")?;
      ss.serialize_field("generic_class_type", &self.with(class_type))?;
ss.serialize_field("generic_argument_list", &self.with(argument_list))?;
      ss.end()
} 
SyntaxVariant::NullableTypeSpecifier (NullableTypeSpecifierChildren{question,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "nullable_type_specifier")?;
      ss.serialize_field("nullable_question", &self.with(question))?;
ss.serialize_field("nullable_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::LikeTypeSpecifier (LikeTypeSpecifierChildren{tilde,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "like_type_specifier")?;
      ss.serialize_field("like_tilde", &self.with(tilde))?;
ss.serialize_field("like_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::SoftTypeSpecifier (SoftTypeSpecifierChildren{at,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "soft_type_specifier")?;
      ss.serialize_field("soft_at", &self.with(at))?;
ss.serialize_field("soft_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::AttributizedSpecifier (AttributizedSpecifierChildren{attribute_spec,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "attributized_specifier")?;
      ss.serialize_field("attributized_specifier_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("attributized_specifier_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::ReifiedTypeArgument (ReifiedTypeArgumentChildren{reified,type_} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "reified_type_argument")?;
      ss.serialize_field("reified_type_argument_reified", &self.with(reified))?;
ss.serialize_field("reified_type_argument_type", &self.with(type_))?;
      ss.end()
} 
SyntaxVariant::TypeArguments (TypeArgumentsChildren{left_angle,types,right_angle} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "type_arguments")?;
      ss.serialize_field("type_arguments_left_angle", &self.with(left_angle))?;
ss.serialize_field("type_arguments_types", &self.with(types))?;
ss.serialize_field("type_arguments_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::TypeParameters (TypeParametersChildren{left_angle,parameters,right_angle} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "type_parameters")?;
      ss.serialize_field("type_parameters_left_angle", &self.with(left_angle))?;
ss.serialize_field("type_parameters_parameters", &self.with(parameters))?;
ss.serialize_field("type_parameters_right_angle", &self.with(right_angle))?;
      ss.end()
} 
SyntaxVariant::TupleTypeSpecifier (TupleTypeSpecifierChildren{left_paren,types,right_paren} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "tuple_type_specifier")?;
      ss.serialize_field("tuple_left_paren", &self.with(left_paren))?;
ss.serialize_field("tuple_types", &self.with(types))?;
ss.serialize_field("tuple_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::UnionTypeSpecifier (UnionTypeSpecifierChildren{left_paren,types,right_paren} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "union_type_specifier")?;
      ss.serialize_field("union_left_paren", &self.with(left_paren))?;
ss.serialize_field("union_types", &self.with(types))?;
ss.serialize_field("union_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::IntersectionTypeSpecifier (IntersectionTypeSpecifierChildren{left_paren,types,right_paren} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "intersection_type_specifier")?;
      ss.serialize_field("intersection_left_paren", &self.with(left_paren))?;
ss.serialize_field("intersection_types", &self.with(types))?;
ss.serialize_field("intersection_right_paren", &self.with(right_paren))?;
      ss.end()
} 
SyntaxVariant::ErrorSyntax (ErrorSyntaxChildren{error} ) => {
      let mut ss = s.serialize_struct("", 2)?;
      ss.serialize_field("kind", "error")?;
      ss.serialize_field("error_error", &self.with(error))?;
      ss.end()
} 
SyntaxVariant::ListItem (ListItemChildren{item,separator} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "list_item")?;
      ss.serialize_field("list_item", &self.with(item))?;
ss.serialize_field("list_separator", &self.with(separator))?;
      ss.end()
} 
SyntaxVariant::EnumClassLabelExpression (EnumClassLabelExpressionChildren{qualifier,hash,expression} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "enum_class_label")?;
      ss.serialize_field("enum_class_label_qualifier", &self.with(qualifier))?;
ss.serialize_field("enum_class_label_hash", &self.with(hash))?;
ss.serialize_field("enum_class_label_expression", &self.with(expression))?;
      ss.end()
} 
SyntaxVariant::ModuleDeclaration (ModuleDeclarationChildren{attribute_spec,new_keyword,module_keyword,name,left_brace,exports,imports,right_brace} ) => {
      let mut ss = s.serialize_struct("", 9)?;
      ss.serialize_field("kind", "module_declaration")?;
      ss.serialize_field("module_declaration_attribute_spec", &self.with(attribute_spec))?;
ss.serialize_field("module_declaration_new_keyword", &self.with(new_keyword))?;
ss.serialize_field("module_declaration_module_keyword", &self.with(module_keyword))?;
ss.serialize_field("module_declaration_name", &self.with(name))?;
ss.serialize_field("module_declaration_left_brace", &self.with(left_brace))?;
ss.serialize_field("module_declaration_exports", &self.with(exports))?;
ss.serialize_field("module_declaration_imports", &self.with(imports))?;
ss.serialize_field("module_declaration_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ModuleExports (ModuleExportsChildren{exports_keyword,left_brace,exports,right_brace} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "module_exports")?;
      ss.serialize_field("module_exports_exports_keyword", &self.with(exports_keyword))?;
ss.serialize_field("module_exports_left_brace", &self.with(left_brace))?;
ss.serialize_field("module_exports_exports", &self.with(exports))?;
ss.serialize_field("module_exports_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ModuleImports (ModuleImportsChildren{imports_keyword,left_brace,imports,right_brace} ) => {
      let mut ss = s.serialize_struct("", 5)?;
      ss.serialize_field("kind", "module_imports")?;
      ss.serialize_field("module_imports_imports_keyword", &self.with(imports_keyword))?;
ss.serialize_field("module_imports_left_brace", &self.with(left_brace))?;
ss.serialize_field("module_imports_imports", &self.with(imports))?;
ss.serialize_field("module_imports_right_brace", &self.with(right_brace))?;
      ss.end()
} 
SyntaxVariant::ModuleMembershipDeclaration (ModuleMembershipDeclarationChildren{module_keyword,name,semicolon} ) => {
      let mut ss = s.serialize_struct("", 4)?;
      ss.serialize_field("kind", "module_membership_declaration")?;
      ss.serialize_field("module_membership_declaration_module_keyword", &self.with(module_keyword))?;
ss.serialize_field("module_membership_declaration_name", &self.with(name))?;
ss.serialize_field("module_membership_declaration_semicolon", &self.with(semicolon))?;
      ss.end()
} 
SyntaxVariant::PackageExpression (PackageExpressionChildren{keyword,name} ) => {
      let mut ss = s.serialize_struct("", 3)?;
      ss.serialize_field("kind", "package_expression")?;
      ss.serialize_field("package_expression_keyword", &self.with(keyword))?;
ss.serialize_field("package_expression_name", &self.with(name))?;
      ss.end()
} 

        }
    }
}
