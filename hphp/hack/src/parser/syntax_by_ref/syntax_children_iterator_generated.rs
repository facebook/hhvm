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
use super::{
    syntax_children_iterator::*,
    syntax_variant_generated::*,
    syntax::*
};

impl<'a, T, V> SyntaxChildrenIterator<'a, T, V> {
    pub fn next_impl(&mut self, direction : bool) -> Option<&'a Syntax<'a, T, V>> {
        use SyntaxVariant::*;
        let get_index = |len| {
            let back_index_plus_1 = len - self.index_back;
            if back_index_plus_1 <= self.index {
                return None
            }
            if direction {
                Some (self.index)
            } else {
                Some (back_index_plus_1 - 1)
            }
        };
        let res = match self.syntax {
            Missing => None,
            Token (_) => None,
            SyntaxList(elems) => {
                get_index(elems.len()).and_then(|x| elems.get(x))
            },
            EndOfFile(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.token),
                        _ => None,
                    }
                })
            },
            Script(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.declarations),
                        _ => None,
                    }
                })
            },
            QualifiedName(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.parts),
                        _ => None,
                    }
                })
            },
            ModuleName(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.parts),
                        _ => None,
                    }
                })
            },
            SimpleTypeSpecifier(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.specifier),
                        _ => None,
                    }
                })
            },
            LiteralExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            PrefixedStringExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.str),
                        _ => None,
                    }
                })
            },
            PrefixedCodeExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.prefix),
                    1 => Some(&x.left_backtick),
                    2 => Some(&x.body),
                    3 => Some(&x.right_backtick),
                        _ => None,
                    }
                })
            },
            VariableExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            PipeVariableExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            FileAttributeSpecification(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.left_double_angle),
                    1 => Some(&x.keyword),
                    2 => Some(&x.colon),
                    3 => Some(&x.attributes),
                    4 => Some(&x.right_double_angle),
                        _ => None,
                    }
                })
            },
            EnumDeclaration(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.keyword),
                    3 => Some(&x.name),
                    4 => Some(&x.colon),
                    5 => Some(&x.base),
                    6 => Some(&x.type_),
                    7 => Some(&x.left_brace),
                    8 => Some(&x.use_clauses),
                    9 => Some(&x.enumerators),
                    10 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            EnumUse(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.names),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            Enumerator(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.equal),
                    2 => Some(&x.value),
                    3 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            EnumClassDeclaration(x) => {
                get_index(12).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.enum_keyword),
                    3 => Some(&x.class_keyword),
                    4 => Some(&x.name),
                    5 => Some(&x.colon),
                    6 => Some(&x.base),
                    7 => Some(&x.extends),
                    8 => Some(&x.extends_list),
                    9 => Some(&x.left_brace),
                    10 => Some(&x.elements),
                    11 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            EnumClassEnumerator(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.modifiers),
                    1 => Some(&x.type_),
                    2 => Some(&x.name),
                    3 => Some(&x.initializer),
                    4 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            AliasDeclaration(x) => {
                get_index(10).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.module_kw_opt),
                    3 => Some(&x.keyword),
                    4 => Some(&x.name),
                    5 => Some(&x.generic_parameter),
                    6 => Some(&x.constraint),
                    7 => Some(&x.equal),
                    8 => Some(&x.type_),
                    9 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ContextAliasDeclaration(x) => {
                get_index(8).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.keyword),
                    2 => Some(&x.name),
                    3 => Some(&x.generic_parameter),
                    4 => Some(&x.as_constraint),
                    5 => Some(&x.equal),
                    6 => Some(&x.context),
                    7 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            CaseTypeDeclaration(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.case_keyword),
                    3 => Some(&x.type_keyword),
                    4 => Some(&x.name),
                    5 => Some(&x.generic_parameter),
                    6 => Some(&x.as_),
                    7 => Some(&x.bounds),
                    8 => Some(&x.equal),
                    9 => Some(&x.variants),
                    10 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            CaseTypeVariant(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.bar),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            PropertyDeclaration(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.type_),
                    3 => Some(&x.declarators),
                    4 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            PropertyDeclarator(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.initializer),
                        _ => None,
                    }
                })
            },
            NamespaceDeclaration(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.header),
                    1 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            NamespaceDeclarationHeader(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            NamespaceBody(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.declarations),
                    2 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            NamespaceEmptyBody(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceUseDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.kind),
                    2 => Some(&x.clauses),
                    3 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceGroupUseDeclaration(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.kind),
                    2 => Some(&x.prefix),
                    3 => Some(&x.left_brace),
                    4 => Some(&x.clauses),
                    5 => Some(&x.right_brace),
                    6 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceUseClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.clause_kind),
                    1 => Some(&x.name),
                    2 => Some(&x.as_),
                    3 => Some(&x.alias),
                        _ => None,
                    }
                })
            },
            FunctionDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.declaration_header),
                    2 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            FunctionDeclarationHeader(x) => {
                get_index(12).and_then(|index| { match index {
                        0 => Some(&x.modifiers),
                    1 => Some(&x.keyword),
                    2 => Some(&x.name),
                    3 => Some(&x.type_parameter_list),
                    4 => Some(&x.left_paren),
                    5 => Some(&x.parameter_list),
                    6 => Some(&x.right_paren),
                    7 => Some(&x.contexts),
                    8 => Some(&x.colon),
                    9 => Some(&x.readonly_return),
                    10 => Some(&x.type_),
                    11 => Some(&x.where_clause),
                        _ => None,
                    }
                })
            },
            Contexts(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_bracket),
                    1 => Some(&x.types),
                    2 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            WhereClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.constraints),
                        _ => None,
                    }
                })
            },
            WhereConstraint(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_type),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_type),
                        _ => None,
                    }
                })
            },
            MethodishDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.attribute),
                    1 => Some(&x.function_decl_header),
                    2 => Some(&x.function_body),
                    3 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            MethodishTraitResolution(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.attribute),
                    1 => Some(&x.function_decl_header),
                    2 => Some(&x.equal),
                    3 => Some(&x.name),
                    4 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ClassishDeclaration(x) => {
                get_index(12).and_then(|index| { match index {
                        0 => Some(&x.attribute),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.xhp),
                    3 => Some(&x.keyword),
                    4 => Some(&x.name),
                    5 => Some(&x.type_parameters),
                    6 => Some(&x.extends_keyword),
                    7 => Some(&x.extends_list),
                    8 => Some(&x.implements_keyword),
                    9 => Some(&x.implements_list),
                    10 => Some(&x.where_clause),
                    11 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            ClassishBody(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.elements),
                    2 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            TraitUse(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.names),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            RequireClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.kind),
                    2 => Some(&x.name),
                    3 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ConstDeclaration(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.keyword),
                    3 => Some(&x.type_specifier),
                    4 => Some(&x.declarators),
                    5 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ConstantDeclarator(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.initializer),
                        _ => None,
                    }
                })
            },
            TypeConstDeclaration(x) => {
                get_index(10).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.modifiers),
                    2 => Some(&x.keyword),
                    3 => Some(&x.type_keyword),
                    4 => Some(&x.name),
                    5 => Some(&x.type_parameters),
                    6 => Some(&x.type_constraints),
                    7 => Some(&x.equal),
                    8 => Some(&x.type_specifier),
                    9 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ContextConstDeclaration(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.modifiers),
                    1 => Some(&x.const_keyword),
                    2 => Some(&x.ctx_keyword),
                    3 => Some(&x.name),
                    4 => Some(&x.type_parameters),
                    5 => Some(&x.constraint),
                    6 => Some(&x.equal),
                    7 => Some(&x.ctx_list),
                    8 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            DecoratedExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.decorator),
                    1 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            ParameterDeclaration(x) => {
                get_index(8).and_then(|index| { match index {
                        0 => Some(&x.attribute),
                    1 => Some(&x.visibility),
                    2 => Some(&x.call_convention),
                    3 => Some(&x.readonly),
                    4 => Some(&x.type_),
                    5 => Some(&x.name),
                    6 => Some(&x.default_value),
                    7 => Some(&x.parameter_end),
                        _ => None,
                    }
                })
            },
            VariadicParameter(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.call_convention),
                    1 => Some(&x.type_),
                    2 => Some(&x.ellipsis),
                        _ => None,
                    }
                })
            },
            OldAttributeSpecification(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_double_angle),
                    1 => Some(&x.attributes),
                    2 => Some(&x.right_double_angle),
                        _ => None,
                    }
                })
            },
            AttributeSpecification(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.attributes),
                        _ => None,
                    }
                })
            },
            Attribute(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.at),
                    1 => Some(&x.attribute_name),
                        _ => None,
                    }
                })
            },
            InclusionExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.require),
                    1 => Some(&x.filename),
                        _ => None,
                    }
                })
            },
            InclusionDirective(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.expression),
                    1 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            CompoundStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.statements),
                    2 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ExpressionStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.expression),
                    1 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            MarkupSection(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.hashbang),
                    1 => Some(&x.suffix),
                        _ => None,
                    }
                })
            },
            MarkupSuffix(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.less_than_question),
                    1 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            UnsetStatement(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.variables),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            DeclareLocalStatement(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.variable),
                    2 => Some(&x.colon),
                    3 => Some(&x.type_),
                    4 => Some(&x.initializer),
                    5 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            UsingStatementBlockScoped(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.await_keyword),
                    1 => Some(&x.using_keyword),
                    2 => Some(&x.left_paren),
                    3 => Some(&x.expressions),
                    4 => Some(&x.right_paren),
                    5 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            UsingStatementFunctionScoped(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.await_keyword),
                    1 => Some(&x.using_keyword),
                    2 => Some(&x.expression),
                    3 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            WhileStatement(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.condition),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            IfStatement(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.condition),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.statement),
                    5 => Some(&x.else_clause),
                        _ => None,
                    }
                })
            },
            ElseClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.statement),
                        _ => None,
                    }
                })
            },
            TryStatement(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.compound_statement),
                    2 => Some(&x.catch_clauses),
                    3 => Some(&x.finally_clause),
                        _ => None,
                    }
                })
            },
            CatchClause(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.type_),
                    3 => Some(&x.variable),
                    4 => Some(&x.right_paren),
                    5 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            FinallyClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            DoStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.body),
                    2 => Some(&x.while_keyword),
                    3 => Some(&x.left_paren),
                    4 => Some(&x.condition),
                    5 => Some(&x.right_paren),
                    6 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ForStatement(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.initializer),
                    3 => Some(&x.first_semicolon),
                    4 => Some(&x.control),
                    5 => Some(&x.second_semicolon),
                    6 => Some(&x.end_of_loop),
                    7 => Some(&x.right_paren),
                    8 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            ForeachStatement(x) => {
                get_index(10).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.collection),
                    3 => Some(&x.await_keyword),
                    4 => Some(&x.as_),
                    5 => Some(&x.key),
                    6 => Some(&x.arrow),
                    7 => Some(&x.value),
                    8 => Some(&x.right_paren),
                    9 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            SwitchStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.expression),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.left_brace),
                    5 => Some(&x.sections),
                    6 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            SwitchSection(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.labels),
                    1 => Some(&x.statements),
                    2 => Some(&x.fallthrough),
                        _ => None,
                    }
                })
            },
            SwitchFallthrough(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            CaseLabel(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.expression),
                    2 => Some(&x.colon),
                        _ => None,
                    }
                })
            },
            DefaultLabel(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.colon),
                        _ => None,
                    }
                })
            },
            MatchStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.expression),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.left_brace),
                    5 => Some(&x.arms),
                    6 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            MatchStatementArm(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.pattern),
                    1 => Some(&x.arrow),
                    2 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            ReturnStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.expression),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            YieldBreakStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.break_),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ThrowStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.expression),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            BreakStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ContinueStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            EchoStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.expressions),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            ConcurrentStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.statement),
                        _ => None,
                    }
                })
            },
            SimpleInitializer(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.equal),
                    1 => Some(&x.value),
                        _ => None,
                    }
                })
            },
            AnonymousClass(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.class_keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.argument_list),
                    3 => Some(&x.right_paren),
                    4 => Some(&x.extends_keyword),
                    5 => Some(&x.extends_list),
                    6 => Some(&x.implements_keyword),
                    7 => Some(&x.implements_list),
                    8 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            AnonymousFunction(x) => {
                get_index(12).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.async_keyword),
                    2 => Some(&x.function_keyword),
                    3 => Some(&x.left_paren),
                    4 => Some(&x.parameters),
                    5 => Some(&x.right_paren),
                    6 => Some(&x.ctx_list),
                    7 => Some(&x.colon),
                    8 => Some(&x.readonly_return),
                    9 => Some(&x.type_),
                    10 => Some(&x.use_),
                    11 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            AnonymousFunctionUseClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.variables),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            VariablePattern(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.variable),
                        _ => None,
                    }
                })
            },
            ConstructorPattern(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.constructor),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.members),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            RefinementPattern(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.variable),
                    1 => Some(&x.colon),
                    2 => Some(&x.specifier),
                        _ => None,
                    }
                })
            },
            LambdaExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.async_),
                    2 => Some(&x.signature),
                    3 => Some(&x.arrow),
                    4 => Some(&x.body),
                        _ => None,
                    }
                })
            },
            LambdaSignature(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.parameters),
                    2 => Some(&x.right_paren),
                    3 => Some(&x.contexts),
                    4 => Some(&x.colon),
                    5 => Some(&x.readonly_return),
                    6 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            CastExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.type_),
                    2 => Some(&x.right_paren),
                    3 => Some(&x.operand),
                        _ => None,
                    }
                })
            },
            ScopeResolutionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.qualifier),
                    1 => Some(&x.operator),
                    2 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            MemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.object),
                    1 => Some(&x.operator),
                    2 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            SafeMemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.object),
                    1 => Some(&x.operator),
                    2 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            EmbeddedMemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.object),
                    1 => Some(&x.operator),
                    2 => Some(&x.name),
                        _ => None,
                    }
                })
            },
            YieldExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.operand),
                        _ => None,
                    }
                })
            },
            PrefixUnaryExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.operator),
                    1 => Some(&x.operand),
                        _ => None,
                    }
                })
            },
            PostfixUnaryExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.operand),
                    1 => Some(&x.operator),
                        _ => None,
                    }
                })
            },
            BinaryExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_operand),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_operand),
                        _ => None,
                    }
                })
            },
            IsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_operand),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_operand),
                        _ => None,
                    }
                })
            },
            AsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_operand),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_operand),
                        _ => None,
                    }
                })
            },
            NullableAsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_operand),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_operand),
                        _ => None,
                    }
                })
            },
            UpcastExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_operand),
                    1 => Some(&x.operator),
                    2 => Some(&x.right_operand),
                        _ => None,
                    }
                })
            },
            ConditionalExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.test),
                    1 => Some(&x.question),
                    2 => Some(&x.consequence),
                    3 => Some(&x.colon),
                    4 => Some(&x.alternative),
                        _ => None,
                    }
                })
            },
            EvalExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.argument),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            IssetExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.argument_list),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            NameofExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.target),
                        _ => None,
                    }
                })
            },
            FunctionCallExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.receiver),
                    1 => Some(&x.type_args),
                    2 => Some(&x.left_paren),
                    3 => Some(&x.argument_list),
                    4 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            FunctionPointerExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.receiver),
                    1 => Some(&x.type_args),
                        _ => None,
                    }
                })
            },
            ParenthesizedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.expression),
                    2 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            BracedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.expression),
                    2 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ETSpliceExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.dollar),
                    1 => Some(&x.left_brace),
                    2 => Some(&x.expression),
                    3 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            EmbeddedBracedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.expression),
                    2 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ListExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.members),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            CollectionLiteralExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.left_brace),
                    2 => Some(&x.initializers),
                    3 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ObjectCreationExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.new_keyword),
                    1 => Some(&x.object),
                        _ => None,
                    }
                })
            },
            ConstructorCall(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.type_),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.argument_list),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            DarrayIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.explicit_type),
                    2 => Some(&x.left_bracket),
                    3 => Some(&x.members),
                    4 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            DictionaryIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.explicit_type),
                    2 => Some(&x.left_bracket),
                    3 => Some(&x.members),
                    4 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            KeysetIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.explicit_type),
                    2 => Some(&x.left_bracket),
                    3 => Some(&x.members),
                    4 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            VarrayIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.explicit_type),
                    2 => Some(&x.left_bracket),
                    3 => Some(&x.members),
                    4 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            VectorIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.explicit_type),
                    2 => Some(&x.left_bracket),
                    3 => Some(&x.members),
                    4 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            ElementInitializer(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.key),
                    1 => Some(&x.arrow),
                    2 => Some(&x.value),
                        _ => None,
                    }
                })
            },
            SubscriptExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.receiver),
                    1 => Some(&x.left_bracket),
                    2 => Some(&x.index),
                    3 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            EmbeddedSubscriptExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.receiver),
                    1 => Some(&x.left_bracket),
                    2 => Some(&x.index),
                    3 => Some(&x.right_bracket),
                        _ => None,
                    }
                })
            },
            AwaitableCreationExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.async_),
                    2 => Some(&x.compound_statement),
                        _ => None,
                    }
                })
            },
            XHPChildrenDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.expression),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            XHPChildrenParenthesizedList(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.xhp_children),
                    2 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            XHPCategoryDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.categories),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            XHPEnumType(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.like),
                    1 => Some(&x.keyword),
                    2 => Some(&x.left_brace),
                    3 => Some(&x.values),
                    4 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            XHPLateinit(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.at),
                    1 => Some(&x.keyword),
                        _ => None,
                    }
                })
            },
            XHPRequired(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.at),
                    1 => Some(&x.keyword),
                        _ => None,
                    }
                })
            },
            XHPClassAttributeDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.attributes),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            XHPClassAttribute(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.type_),
                    1 => Some(&x.name),
                    2 => Some(&x.initializer),
                    3 => Some(&x.required),
                        _ => None,
                    }
                })
            },
            XHPSimpleClassAttribute(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            XHPSimpleAttribute(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.equal),
                    2 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            XHPSpreadAttribute(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.left_brace),
                    1 => Some(&x.spread_operator),
                    2 => Some(&x.expression),
                    3 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            XHPOpen(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.left_angle),
                    1 => Some(&x.name),
                    2 => Some(&x.attributes),
                    3 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            XHPExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.open),
                    1 => Some(&x.body),
                    2 => Some(&x.close),
                        _ => None,
                    }
                })
            },
            XHPClose(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_angle),
                    1 => Some(&x.name),
                    2 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            TypeConstant(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_type),
                    1 => Some(&x.separator),
                    2 => Some(&x.right_type),
                        _ => None,
                    }
                })
            },
            VectorTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.type_),
                    3 => Some(&x.trailing_comma),
                    4 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            KeysetTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.type_),
                    3 => Some(&x.trailing_comma),
                    4 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            TupleTypeExplicitSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.types),
                    3 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            VarrayTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.type_),
                    3 => Some(&x.trailing_comma),
                    4 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            FunctionCtxTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.variable),
                        _ => None,
                    }
                })
            },
            TypeParameter(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.reified),
                    2 => Some(&x.variance),
                    3 => Some(&x.name),
                    4 => Some(&x.param_params),
                    5 => Some(&x.constraints),
                        _ => None,
                    }
                })
            },
            TypeConstraint(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            ContextConstraint(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.ctx_list),
                        _ => None,
                    }
                })
            },
            DarrayTypeSpecifier(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.key),
                    3 => Some(&x.comma),
                    4 => Some(&x.value),
                    5 => Some(&x.trailing_comma),
                    6 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            DictionaryTypeSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.members),
                    3 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            ClosureTypeSpecifier(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.outer_left_paren),
                    1 => Some(&x.readonly_keyword),
                    2 => Some(&x.function_keyword),
                    3 => Some(&x.inner_left_paren),
                    4 => Some(&x.parameter_list),
                    5 => Some(&x.inner_right_paren),
                    6 => Some(&x.contexts),
                    7 => Some(&x.colon),
                    8 => Some(&x.readonly_return),
                    9 => Some(&x.return_type),
                    10 => Some(&x.outer_right_paren),
                        _ => None,
                    }
                })
            },
            ClosureParameterTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.call_convention),
                    1 => Some(&x.readonly),
                    2 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            TypeRefinement(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.type_),
                    1 => Some(&x.keyword),
                    2 => Some(&x.left_brace),
                    3 => Some(&x.members),
                    4 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            TypeInRefinement(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.name),
                    2 => Some(&x.type_parameters),
                    3 => Some(&x.constraints),
                    4 => Some(&x.equal),
                    5 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            CtxInRefinement(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.name),
                    2 => Some(&x.type_parameters),
                    3 => Some(&x.constraints),
                    4 => Some(&x.equal),
                    5 => Some(&x.ctx_list),
                        _ => None,
                    }
                })
            },
            ClassnameTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.type_),
                    3 => Some(&x.trailing_comma),
                    4 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            ClassArgsTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_angle),
                    2 => Some(&x.type_),
                    3 => Some(&x.trailing_comma),
                    4 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            FieldSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.question),
                    1 => Some(&x.name),
                    2 => Some(&x.arrow),
                    3 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            FieldInitializer(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.name),
                    1 => Some(&x.arrow),
                    2 => Some(&x.value),
                        _ => None,
                    }
                })
            },
            ShapeTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.fields),
                    3 => Some(&x.ellipsis),
                    4 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            ShapeExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.fields),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            TupleExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.left_paren),
                    2 => Some(&x.items),
                    3 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            GenericTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.class_type),
                    1 => Some(&x.argument_list),
                        _ => None,
                    }
                })
            },
            NullableTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.question),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            LikeTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.tilde),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            SoftTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.at),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            AttributizedSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            ReifiedTypeArgument(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.reified),
                    1 => Some(&x.type_),
                        _ => None,
                    }
                })
            },
            TypeArguments(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_angle),
                    1 => Some(&x.types),
                    2 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            TypeParameters(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_angle),
                    1 => Some(&x.parameters),
                    2 => Some(&x.right_angle),
                        _ => None,
                    }
                })
            },
            TupleTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.types),
                    2 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            UnionTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.types),
                    2 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            IntersectionTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.left_paren),
                    1 => Some(&x.types),
                    2 => Some(&x.right_paren),
                        _ => None,
                    }
                })
            },
            ErrorSyntax(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.error),
                        _ => None,
                    }
                })
            },
            ListItem(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.item),
                    1 => Some(&x.separator),
                        _ => None,
                    }
                })
            },
            EnumClassLabelExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.qualifier),
                    1 => Some(&x.hash),
                    2 => Some(&x.expression),
                        _ => None,
                    }
                })
            },
            ModuleDeclaration(x) => {
                get_index(8).and_then(|index| { match index {
                        0 => Some(&x.attribute_spec),
                    1 => Some(&x.new_keyword),
                    2 => Some(&x.module_keyword),
                    3 => Some(&x.name),
                    4 => Some(&x.left_brace),
                    5 => Some(&x.exports),
                    6 => Some(&x.imports),
                    7 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ModuleExports(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.exports_keyword),
                    1 => Some(&x.left_brace),
                    2 => Some(&x.exports),
                    3 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ModuleImports(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.imports_keyword),
                    1 => Some(&x.left_brace),
                    2 => Some(&x.imports),
                    3 => Some(&x.right_brace),
                        _ => None,
                    }
                })
            },
            ModuleMembershipDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.module_keyword),
                    1 => Some(&x.name),
                    2 => Some(&x.semicolon),
                        _ => None,
                    }
                })
            },
            PackageExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.keyword),
                    1 => Some(&x.name),
                        _ => None,
                    }
                })
            },

        };
        if res.is_some() {
            if direction {
                self.index = self.index + 1
            } else {
                self.index_back = self.index_back + 1
            }
        }
        res
    }
}
    