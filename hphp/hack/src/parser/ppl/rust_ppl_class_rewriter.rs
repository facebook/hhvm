// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::{
    syntax::{
        PrefixUnaryExpressionChildren, Syntax, SyntaxTypeBase, SyntaxValueType, SyntaxVariant,
    },
    syntax_type::SyntaxType,
};
use rewriter::Rewriter;
use rust_coroutine::CoroutineSyntax;
use rust_editable_positioned_syntax::{
    editable_positioned_token::EditablePositionedToken, EditablePositionedSyntax,
    EditablePositionedSyntaxTrait, EditablePositionedValue,
};

const RECEIVER_STR: &str = "$__recv";
const PPL_INFER_TYPE_STR: &str = "Infer";
const PPL_MACRO_STR: &str = "__PPL";

pub struct PplClassRewriter {}

impl<'a> PplClassRewriter {
    fn add_coroutine_modifier_to_modifiers_list(
        function_modifiers: &EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        if CoroutineSyntax::has_coroutine_modifier(function_modifiers) {
            panic!("Should not lower methods that are already coroutines");
        } else {
            let coroutine_token_syntax = CoroutineSyntax::make_coroutine_token_syntax();
            let mut modifiers_list = Self::vecref_to_vec(
                Syntax::syntax_node_to_list(function_modifiers).collect::<Vec<_>>(),
            );
            modifiers_list.insert(0, coroutine_token_syntax);
            Syntax::make_list(&(), modifiers_list, 0)
        }
    }

    fn add_infer_receiver_to_parameter_list(
        parameters: &EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        let ppl_infer_type_name_syntax =
            CoroutineSyntax::make_qualified_name_syntax(&vec![PPL_INFER_TYPE_STR], true);
        let infer_type_parameter_syntax =
            SyntaxType::make_simple_type_specifier(&(), ppl_infer_type_name_syntax);
        let infer_parameter_syntax = CoroutineSyntax::make_parameter_declaration_syntax(
            Syntax::make_missing(&(), 0),
            infer_type_parameter_syntax,
            String::from(RECEIVER_STR),
        );
        CoroutineSyntax::prepend_to_comma_delimited_syntax_list(infer_parameter_syntax, parameters)
    }

    fn rewrite_ppl_method_header(method_header: &mut EditablePositionedSyntax<'a>) -> () {
        match &mut method_header.syntax {
            SyntaxVariant::FunctionDeclarationHeader(declaration_header) => {
                declaration_header.function_modifiers =
                    Self::add_coroutine_modifier_to_modifiers_list(
                        &declaration_header.function_modifiers,
                    );
                declaration_header.function_parameter_list =
                    Self::add_infer_receiver_to_parameter_list(
                        &declaration_header.function_parameter_list,
                    );
            }
            _ => panic!("Expected function declaration header"),
        }
    }

    fn should_be_rewritten(receiver: &EditablePositionedSyntax<'a>) -> bool {
        match &receiver.syntax {
            SyntaxVariant::MemberSelectionExpression(x) => Self::is_this(&x.member_object),
            SyntaxVariant::ScopeResolutionExpression(x) => {
                match Syntax::get_token(&x.scope_resolution_qualifier) {
                    Some(qualifier) => {
                        let name = Syntax::get_token(&x.scope_resolution_name).unwrap();
                        let qualifier_text = EditablePositionedToken::text(qualifier);
                        let name_text = EditablePositionedToken::text(name);
                        (qualifier_text == "parent" && name_text != "__construct")
                            || qualifier_text == "static"
                            || qualifier_text == "self"
                    }
                    None => Self::is_this(&x.scope_resolution_qualifier),
                }
            }
            _ => false,
        }
    }

    fn rewrite_ppl_method_body(
        method_body: EditablePositionedSyntax<'a>,
        suspension_id: usize,
    ) -> EditablePositionedSyntax<'a> {
        Rewriter::aggregating_rewrite_post(method_body, suspension_id, Self::rewrite_function_call)
            .0
    }

    fn rewrite_function_call(
        function_call: EditablePositionedSyntax<'a>,
        suspension_id: &mut usize,
    ) -> (Option<EditablePositionedSyntax<'a>>, bool) {
        if let SyntaxVariant::FunctionCallExpression(mut x) = function_call.syntax {
            if Self::should_be_rewritten(&x.function_call_receiver) {
                x.function_call_argument_list =
                    CoroutineSyntax::prepend_to_comma_delimited_syntax_list(
                        Self::make_receiver_variable_syntax(),
                        &x.function_call_argument_list,
                    );
                let new_function_call = Syntax::make(
                    SyntaxVariant::FunctionCallExpression(x),
                    function_call.value,
                );
                let new_syntax = CoroutineSyntax::make_syntax(
                    SyntaxVariant::PrefixUnaryExpression(Box::new(PrefixUnaryExpressionChildren {
                        prefix_unary_operator: CoroutineSyntax::make_suspend_token_syntax(),
                        prefix_unary_operand: new_function_call,
                    })),
                );
                (Some(new_syntax), true)
            } else {
                let text = EditablePositionedSyntax::text(&x.function_call_receiver);
                let text = text.trim_start_matches("\\");
                if Self::is_infer_method(text) {
                    x.function_call_receiver =
                        CoroutineSyntax::make_member_selection_expression_syntax(
                            Self::make_receiver_variable_syntax(),
                            CoroutineSyntax::make_name_syntax(text.to_string()),
                        );
                    let new_function_call = Syntax::make(
                        SyntaxVariant::FunctionCallExpression(x),
                        function_call.value,
                    );
                    let new_syntax =
                        CoroutineSyntax::make_syntax(SyntaxVariant::PrefixUnaryExpression(
                            Box::new(PrefixUnaryExpressionChildren {
                                prefix_unary_operator: CoroutineSyntax::make_suspend_token_syntax(),
                                prefix_unary_operand: new_function_call,
                            }),
                        ));
                    *suspension_id += 1;
                    (Some(new_syntax), true)
                } else {
                    let old_function_call = Syntax::make(
                        SyntaxVariant::FunctionCallExpression(x),
                        function_call.value,
                    );
                    (Some(old_function_call), false)
                }
            }
        } else {
            (Some(function_call), false)
        }
    }

    fn is_ppl_attribute(attribute: &EditablePositionedSyntax<'a>) -> bool {
        match &attribute.syntax {
            SyntaxVariant::ListItem(x) => match &(&x.list_item).syntax {
                SyntaxVariant::ConstructorCall(y) => {
                    let token_text = Syntax::get_token(&y.constructor_call_type).unwrap();
                    EditablePositionedToken::text(token_text) == PPL_MACRO_STR
                }
                _ => false,
            },
            _ => false,
        }
    }

    fn rewrite_ppl_class_body(
        classish_body: EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        Rewriter::rewrite_pre_and_stop_with_acc(classish_body, 0, Self::rewrite_ppl_class_method).0
    }

    fn rewrite_ppl_class_method(
        class_method: EditablePositionedSyntax<'a>,
        acc: &mut usize,
    ) -> (Option<EditablePositionedSyntax<'a>>, bool) {
        use SyntaxVariant::*;
        if let MethodishDeclaration(x) = &class_method.syntax {
            if let FunctionDeclarationHeader(y) = &x.methodish_function_decl_header.syntax {
                if !CoroutineSyntax::has_coroutine_modifier(&y.function_modifiers)
                    && !Syntax::is_construct(&y.function_name)
                {
                    if let MethodishDeclaration(mut x) = class_method.syntax {
                        Self::rewrite_ppl_method_header(&mut x.methodish_function_decl_header);
                        x.methodish_function_body =
                            Self::rewrite_ppl_method_body(x.methodish_function_body, *acc);
                        let newnode = Syntax::make(
                            SyntaxVariant::MethodishDeclaration(x),
                            class_method.value,
                        );
                        return (Some(newnode), true);
                    }
                }
            }
        }

        (Some(class_method), false)
    }

    fn has_ppl_attribute(attributes: &EditablePositionedSyntax<'a>) -> bool {
        match &attributes.syntax {
            SyntaxVariant::OldAttributeSpecification(x) => {
                Syntax::syntax_node_to_list(&x.old_attribute_specification_attributes)
                    .any(|attr| Self::is_ppl_attribute(attr))
            }
            SyntaxVariant::AttributeSpecification(x) => {
                Syntax::syntax_node_to_list(&x.attribute_specification_attributes)
                    .any(|attr| Self::is_ppl_attribute(attr))
            }
            _ => false,
        }
    }

    fn remove_ppl_attribute(
        attributes: EditablePositionedSyntax<'a>,
    ) -> EditablePositionedSyntax<'a> {
        match attributes.syntax {
            SyntaxVariant::OldAttributeSpecification(mut old_attr_spec) => {
                let attribute_list = Syntax::syntax_node_to_list(
                    &old_attr_spec.old_attribute_specification_attributes,
                );
                let new_attribute_list = Self::vecref_to_vec(
                    attribute_list
                        .filter(|attr| !Self::is_ppl_attribute(attr))
                        .collect::<Vec<_>>(),
                );
                if new_attribute_list.is_empty() {
                    Syntax::make_missing(&(), 0)
                } else {
                    old_attr_spec.old_attribute_specification_attributes =
                        Syntax::make_list(&(), new_attribute_list, 0);
                    Syntax::make(
                        SyntaxVariant::OldAttributeSpecification(old_attr_spec),
                        attributes.value,
                    )
                }
            }
            SyntaxVariant::AttributeSpecification(mut attr_spec) => {
                let attribute_list =
                    Syntax::syntax_node_to_list(&attr_spec.attribute_specification_attributes);
                let new_attribute_list = Self::vecref_to_vec(
                    attribute_list
                        .filter(|attr| !Self::is_ppl_attribute(attr))
                        .collect::<Vec<_>>(),
                );
                if new_attribute_list.is_empty() {
                    Syntax::make_missing(&(), 0)
                } else {
                    attr_spec.attribute_specification_attributes =
                        Syntax::make_list(&(), new_attribute_list, 0);
                    Syntax::make(
                        SyntaxVariant::AttributeSpecification(attr_spec),
                        attributes.value,
                    )
                }
            }
            _ => panic!("Expected Attribute Specification"),
        }
    }

    fn rewrite_all_declarations<Nodes>(
        declaration_list: Nodes,
    ) -> impl DoubleEndedIterator<Item = EditablePositionedSyntax<'a>>
    where
        Nodes: DoubleEndedIterator<Item = EditablePositionedSyntax<'a>>,
    {
        declaration_list.map(|decl| Rewriter::rewrite_pre_and_stop(decl, Self::rewrite_declaration))
    }

    pub fn rewrite_ppl_classes(root: &mut EditablePositionedSyntax<'a>) -> () {
        use std::iter::once;
        match &mut root.syntax {
            SyntaxVariant::Script(x) => {
                let mut declarations = Syntax::syntax_node_into_list(x.script_declarations.clone());
                match declarations.next() {
                    Some(hh_decl) => {
                        let new_nodes = Self::rewrite_all_declarations(declarations);
                        let declarations = once(hh_decl).chain(new_nodes);
                        let new_script_declarations = declarations.collect::<Vec<_>>();
                        x.script_declarations = Syntax::make_list(&(), new_script_declarations, 0);
                        root.value = EditablePositionedValue::from_syntax(&root.syntax);
                    }
                    None => panic!("How did we get a script with no header element?"),
                }
            }
            _ => panic!("How did we get a root that is not a script?"),
        }
    }

    fn rewrite_declaration(
        declaration: EditablePositionedSyntax<'a>,
    ) -> (Option<EditablePositionedSyntax<'a>>, bool) {
        if let SyntaxVariant::ClassishDeclaration(mut x) = declaration.syntax {
            if Self::has_ppl_attribute(&x.classish_attribute) {
                x.classish_body = Self::rewrite_ppl_class_body(x.classish_body);
                x.classish_attribute = Self::remove_ppl_attribute(x.classish_attribute);
            }
            let new_declaration =
                Syntax::make(SyntaxVariant::ClassishDeclaration(x), declaration.value);
            (Some(new_declaration), true)
        } else {
            (Some(declaration), false)
        }
    }

    fn is_this(preface: &EditablePositionedSyntax<'a>) -> bool {
        match &preface.syntax {
            SyntaxVariant::VariableExpression(x) => {
                let token = Syntax::get_token(&x.variable_expression).unwrap();
                EditablePositionedToken::text(token) == "$this"
            }
            SyntaxVariant::ParenthesizedExpression(x) => {
                Self::is_this(&x.parenthesized_expression_expression)
            }
            _ => false,
        }
    }

    fn vecref_to_vec<T: Clone>(r: Vec<&T>) -> Vec<T> {
        let mut v = vec![];
        for e in r {
            v.push(e.clone());
        }
        v
    }

    fn make_receiver_variable_syntax() -> EditablePositionedSyntax<'a> {
        let receiver_string = String::from(RECEIVER_STR);
        CoroutineSyntax::make_variable_expression_syntax(receiver_string)
    }

    fn is_infer_method(method_string: &str) -> bool {
        let reserved_method_names = ["sample", "sample_model", "factor", "observe", "condition"];
        reserved_method_names.contains(&method_string)
    }
}
