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
use crate::{OcamlSyntax, Context};
use rust_to_ocaml::*;

use parser_core_types::syntax_kind::SyntaxKind;
use parser_core_types::syntax::{SyntaxType, SyntaxValueType};
use parser_core_types::positioned_token::PositionedToken;

impl<V, C> SyntaxType<C> for OcamlSyntax<V>
where
    C: Context,
    V: SyntaxValueType<PositionedToken> + ToOcaml,
{
    fn make_end_of_file(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EndOfFile,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_script(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::Script,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_qualified_name(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::QualifiedName,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_simple_type_specifier(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SimpleTypeSpecifier,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_literal_expression(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::LiteralExpression,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_prefixed_string_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PrefixedStringExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_prefixed_code_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PrefixedCodeExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_variable_expression(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VariableExpression,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_pipe_variable_expression(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PipeVariableExpression,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_file_attribute_specification(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FileAttributeSpecification,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enum_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EnumDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enum_use(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EnumUse,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enumerator(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::Enumerator,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enum_class_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self, arg10: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value, 
          arg10.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EnumClassDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax, 
              arg10.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enum_class_enumerator(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EnumClassEnumerator,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_record_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::RecordDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_record_field(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::RecordField,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_alias_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AliasDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_property_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PropertyDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_property_declarator(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PropertyDeclarator,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_declaration(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_declaration_header(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceDeclarationHeader,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_body(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceBody,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_empty_body(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceEmptyBody,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_use_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceUseDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_group_use_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceGroupUseDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_namespace_use_clause(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NamespaceUseClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_function_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FunctionDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_function_declaration_header(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self, arg10: Self, arg11: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value, 
          arg10.value, 
          arg11.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FunctionDeclarationHeader,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax, 
              arg10.syntax, 
              arg11.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_contexts(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::Contexts,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_where_clause(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::WhereClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_where_constraint(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::WhereConstraint,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_methodish_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::MethodishDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_methodish_trait_resolution(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::MethodishTraitResolution,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_classish_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self, arg10: Self, arg11: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value, 
          arg10.value, 
          arg11.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ClassishDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax, 
              arg10.syntax, 
              arg11.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_classish_body(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ClassishBody,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_trait_use_precedence_item(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TraitUsePrecedenceItem,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_trait_use_alias_item(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TraitUseAliasItem,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_trait_use_conflict_resolution(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TraitUseConflictResolution,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_trait_use(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TraitUse,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_require_clause(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::RequireClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_const_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ConstDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_constant_declarator(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ConstantDeclarator,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_const_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeConstDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_context_const_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ContextConstDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_decorated_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DecoratedExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_parameter_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ParameterDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_variadic_parameter(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VariadicParameter,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_old_attribute_specification(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::OldAttributeSpecification,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_attribute_specification(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AttributeSpecification,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_attribute(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::Attribute,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_inclusion_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::InclusionExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_inclusion_directive(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::InclusionDirective,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_compound_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::CompoundStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_expression_statement(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ExpressionStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_markup_section(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::MarkupSection,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_markup_suffix(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::MarkupSuffix,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_unset_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::UnsetStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_using_statement_block_scoped(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::UsingStatementBlockScoped,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_using_statement_function_scoped(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::UsingStatementFunctionScoped,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_while_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::WhileStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_if_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::IfStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_elseif_clause(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ElseifClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_else_clause(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ElseClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_try_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TryStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_catch_clause(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::CatchClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_finally_clause(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FinallyClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_do_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DoStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_for_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ForStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_foreach_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ForeachStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_switch_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SwitchStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_switch_section(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SwitchSection,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_switch_fallthrough(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SwitchFallthrough,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_case_label(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::CaseLabel,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_default_label(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DefaultLabel,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_return_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ReturnStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_yield_break_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::YieldBreakStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_throw_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ThrowStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_break_statement(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::BreakStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_continue_statement(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ContinueStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_echo_statement(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EchoStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_concurrent_statement(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ConcurrentStatement,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_simple_initializer(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SimpleInitializer,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_anonymous_class(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AnonymousClass,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_anonymous_function(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self, arg10: Self, arg11: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value, 
          arg10.value, 
          arg11.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AnonymousFunction,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax, 
              arg10.syntax, 
              arg11.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_anonymous_function_use_clause(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AnonymousFunctionUseClause,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_lambda_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::LambdaExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_lambda_signature(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::LambdaSignature,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_cast_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::CastExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_scope_resolution_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ScopeResolutionExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_member_selection_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::MemberSelectionExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_safe_member_selection_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SafeMemberSelectionExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_embedded_member_selection_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EmbeddedMemberSelectionExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_yield_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::YieldExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_prefix_unary_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PrefixUnaryExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_postfix_unary_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::PostfixUnaryExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_binary_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::BinaryExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_is_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::IsExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_as_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AsExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_nullable_as_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NullableAsExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_conditional_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ConditionalExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_eval_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EvalExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_define_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DefineExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_isset_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::IssetExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_function_call_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FunctionCallExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_function_pointer_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FunctionPointerExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_parenthesized_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ParenthesizedExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_braced_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::BracedExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_et_splice_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ETSpliceExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_embedded_braced_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EmbeddedBracedExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_list_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ListExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_collection_literal_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::CollectionLiteralExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_object_creation_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ObjectCreationExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_constructor_call(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ConstructorCall,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_record_creation_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::RecordCreationExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_darray_intrinsic_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DarrayIntrinsicExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_dictionary_intrinsic_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DictionaryIntrinsicExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_keyset_intrinsic_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::KeysetIntrinsicExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_varray_intrinsic_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VarrayIntrinsicExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_vector_intrinsic_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VectorIntrinsicExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_element_initializer(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ElementInitializer,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_subscript_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SubscriptExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_embedded_subscript_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EmbeddedSubscriptExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_awaitable_creation_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AwaitableCreationExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_children_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPChildrenDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_children_parenthesized_list(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPChildrenParenthesizedList,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_category_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPCategoryDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_enum_type(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPEnumType,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_lateinit(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPLateinit,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_required(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPRequired,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_class_attribute_declaration(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPClassAttributeDeclaration,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_class_attribute(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPClassAttribute,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_simple_class_attribute(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPSimpleClassAttribute,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_simple_attribute(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPSimpleAttribute,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_spread_attribute(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPSpreadAttribute,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_open(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPOpen,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_xhp_close(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::XHPClose,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_constant(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeConstant,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_vector_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VectorTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_keyset_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::KeysetTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_tuple_type_explicit_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TupleTypeExplicitSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_varray_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::VarrayTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_function_ctx_type_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FunctionCtxTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_parameter(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeParameter,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_constraint(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeConstraint,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_context_constraint(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ContextConstraint,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_darray_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DarrayTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_dictionary_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::DictionaryTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_closure_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self, arg5: Self, arg6: Self, arg7: Self, arg8: Self, arg9: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value, 
          arg5.value, 
          arg6.value, 
          arg7.value, 
          arg8.value, 
          arg9.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ClosureTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax, 
              arg5.syntax, 
              arg6.syntax, 
              arg7.syntax, 
              arg8.syntax, 
              arg9.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_closure_parameter_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ClosureParameterTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_classname_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ClassnameTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_field_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FieldSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_field_initializer(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::FieldInitializer,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_shape_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self, arg4: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value, 
          arg4.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ShapeTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax, 
              arg4.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_shape_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ShapeExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_tuple_expression(ctx: &C, arg0: Self, arg1: Self, arg2: Self, arg3: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value, 
          arg3.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TupleExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax, 
              arg3.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_generic_type_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::GenericTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_nullable_type_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::NullableTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_like_type_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::LikeTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_soft_type_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::SoftTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_attributized_specifier(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::AttributizedSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_reified_type_argument(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ReifiedTypeArgument,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_arguments(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeArguments,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_type_parameters(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TypeParameters,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_tuple_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::TupleTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_union_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::UnionTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_intersection_type_specifier(ctx: &C, arg0: Self, arg1: Self, arg2: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value, 
          arg2.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::IntersectionTypeSpecifier,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax, 
              arg2.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_error(ctx: &C, arg0: Self) -> Self {
      let children = &[
          arg0.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ErrorSyntax,
          &value,
          &[
              arg0.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_list_item(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::ListItem,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

    fn make_enum_atom_expression(ctx: &C, arg0: Self, arg1: Self) -> Self {
      let children = &[
          arg0.value, 
          arg1.value
      ];
      let value = V::from_values(children.iter());
      let syntax = Self::make(
          ctx,
          SyntaxKind::EnumAtomExpression,
          &value,
          &[
              arg0.syntax, 
              arg1.syntax
          ],
      );
      Self { syntax, value }
    }

}
  