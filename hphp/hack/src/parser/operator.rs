/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::parser_env::ParserEnv;
use crate::token_kind::TokenKind;

pub enum Operator {
    DollarOperator,
    /* TODO: Is there a better name? Operators should be named as what they do,
    not how they look on the page. */
    IndexingOperator,
    FunctionCallOperator,
    AwaitOperator,
    SuspendOperator,
    PipeOperator,
    ConditionalQuestionOperator,
    ConditionalColonOperator,
    DegenerateConditionalOperator,
    CoalesceOperator,
    CoalesceAssignmentOperator,
    PHPOrOperator,
    PHPExclusiveOrOperator,
    PHPAndOperator,
    PrintOperator,
    LogicalOrOperator,
    ExclusiveOrOperator,
    LogicalAndOperator,
    OrOperator,
    AndOperator,
    EqualOperator,
    StrictEqualOperator,
    NotEqualOperator,
    PhpNotEqualOperator,
    StrictNotEqualOperator,
    SpaceshipOperator,
    LessThanOperator,
    LessThanOrEqualOperator,
    GreaterThanOperator,
    GreaterThanOrEqualOperator,
    LeftShiftOperator,
    RightShiftOperator,
    AdditionOperator,
    SubtractionOperator,
    ConcatenationOperator,
    MultiplicationOperator,
    DivisionOperator,
    RemainderOperator,
    LogicalNotOperator,
    InstanceofOperator,
    IsOperator,
    AsOperator,
    NullableAsOperator,
    NotOperator,
    PrefixIncrementOperator,
    PrefixDecrementOperator,
    PostfixIncrementOperator,
    PostfixDecrementOperator,
    CastOperator,
    ExponentOperator,
    ReferenceOperator,
    ErrorControlOperator,
    NewOperator,
    CloneOperator,
    AssignmentOperator,
    AdditionAssignmentOperator,
    SubtractionAssignmentOperator,
    MultiplicationAssignmentOperator,
    DivisionAssignmentOperator,
    ExponentiationAssignmentOperator,
    ConcatenationAssignmentOperator,
    RemainderAssignmentOperator,
    AndAssignmentOperator,
    OrAssignmentOperator,
    ExclusiveOrAssignmentOperator,
    LeftShiftAssignmentOperator,
    RightShiftAssignmentOperator,
    MemberSelectionOperator,
    NullSafeMemberSelectionOperator,
    ScopeResolutionOperator,
    UnaryPlusOperator,
    UnaryMinusOperator,
    IncludeOperator,
    IncludeOnceOperator,
    RequireOperator,
    RequireOnceOperator,
}

#[derive(PartialEq)]
pub enum Assoc {
    LeftAssociative,
    RightAssociative,
    NotAssociative,
}

use self::Operator::*;

impl Operator {
    pub fn precedence(&self, env: &ParserEnv) -> usize {
        /* TODO: eval */
        /* TODO: Comma */
        /* TODO: elseif */
        /* TODO: else */
        /* TODO: endif */
        /* TODO: variable operator $ */
        match self {
            | IncludeOperator | IncludeOnceOperator | RequireOperator
            | RequireOnceOperator => 1,
            | AwaitOperator if !env.enable_stronger_await_binding => 1,
            | PHPOrOperator => 2,
            | PHPExclusiveOrOperator => 3,
            | PHPAndOperator => 4,
            | PrintOperator => 5,
            | AssignmentOperator | AdditionAssignmentOperator
            | SubtractionAssignmentOperator | MultiplicationAssignmentOperator
            | DivisionAssignmentOperator | ExponentiationAssignmentOperator
            | RemainderAssignmentOperator | ConcatenationAssignmentOperator
            | AndAssignmentOperator
            | OrAssignmentOperator | ExclusiveOrAssignmentOperator
            | LeftShiftAssignmentOperator | RightShiftAssignmentOperator
            | CoalesceAssignmentOperator
                => 6,
            | PipeOperator => 7,
            | ConditionalQuestionOperator | ConditionalColonOperator
            | DegenerateConditionalOperator => 8,
            | CoalesceOperator => 9,
            | LogicalOrOperator => 10,
            | LogicalAndOperator => 11,
            | OrOperator => 12,
            | ExclusiveOrOperator => 13,
            | AndOperator => 14,
            | EqualOperator | StrictEqualOperator
            | PhpNotEqualOperator | NotEqualOperator | StrictNotEqualOperator => 15,
            | SpaceshipOperator | LessThanOperator | LessThanOrEqualOperator
            | GreaterThanOperator | GreaterThanOrEqualOperator => 16,
            | LeftShiftOperator | RightShiftOperator => 17,
            | AdditionOperator | SubtractionOperator | ConcatenationOperator => 18,
            | MultiplicationOperator | DivisionOperator | RemainderOperator | SuspendOperator => 19,
            | LogicalNotOperator| NotOperator
            | UnaryPlusOperator | UnaryMinusOperator => 20,
            | InstanceofOperator | IsOperator | AsOperator | NullableAsOperator => 21,
            | CastOperator
            | ErrorControlOperator
            | PrefixIncrementOperator | PrefixDecrementOperator
            | ExponentOperator => 22,
            | PostfixIncrementOperator | PostfixDecrementOperator => 23,
            | AwaitOperator /* implicit: when Env.enable_stronger_await_binding env */
                => 23,
            | CloneOperator => 24,
            /* value 25 is reserved for assignment that appear in expressions */
            | ReferenceOperator => 26,
            | FunctionCallOperator => 27,
            | NewOperator => 28,
            | MemberSelectionOperator | NullSafeMemberSelectionOperator => 29,
            | IndexingOperator => 30,
            | ScopeResolutionOperator => 31,
            | DollarOperator => 32,
        }
    }

    pub fn precedence_for_assignment_in_expressions() -> usize {
        25
    }

    pub fn associativity(&self, env: &ParserEnv) -> Assoc {
        match self {
            | EqualOperator | StrictEqualOperator | NotEqualOperator | PhpNotEqualOperator
            | StrictNotEqualOperator | LessThanOperator | LessThanOrEqualOperator
            | GreaterThanOperator | GreaterThanOrEqualOperator | InstanceofOperator
            | NewOperator | CloneOperator | SpaceshipOperator => Assoc::NotAssociative,
            | AwaitOperator if !env.enable_stronger_await_binding
                => Assoc::NotAssociative,
            | DegenerateConditionalOperator
            | PipeOperator | ConditionalQuestionOperator | ConditionalColonOperator
            | LogicalOrOperator | ExclusiveOrOperator | LogicalAndOperator
            | OrOperator | AndOperator | LeftShiftOperator | RightShiftOperator
            | AdditionOperator | SubtractionOperator | ConcatenationOperator
            | MultiplicationOperator | DivisionOperator | RemainderOperator
            | MemberSelectionOperator | NullSafeMemberSelectionOperator
            | ScopeResolutionOperator | FunctionCallOperator | IndexingOperator
            | IncludeOperator | IncludeOnceOperator | RequireOperator
            | RequireOnceOperator | PHPAndOperator | PHPOrOperator
            | PHPExclusiveOrOperator | IsOperator | AsOperator | NullableAsOperator
                /* eval */
                /* Comma */
                /* elseif */
                /* else */
                /* endif */
                => Assoc::LeftAssociative,
            | CoalesceOperator | CoalesceAssignmentOperator | LogicalNotOperator | NotOperator | CastOperator
            | DollarOperator | UnaryPlusOperator | UnaryMinusOperator  /* TODO: Correct? */
            | ErrorControlOperator | ReferenceOperator /* TODO: Correct? */
            | PostfixIncrementOperator | PostfixDecrementOperator
            | PrefixIncrementOperator | PrefixDecrementOperator | ExponentOperator
            | AssignmentOperator | AdditionAssignmentOperator
            | SubtractionAssignmentOperator | MultiplicationAssignmentOperator
            | DivisionAssignmentOperator | ExponentiationAssignmentOperator
            | ConcatenationAssignmentOperator
            | RemainderAssignmentOperator | AndAssignmentOperator
            | OrAssignmentOperator | ExclusiveOrAssignmentOperator
            | LeftShiftAssignmentOperator | RightShiftAssignmentOperator
            | PrintOperator | SuspendOperator => Assoc::RightAssociative,
            | AwaitOperator /* implicitly: Env.enable_stronger_await_binding env*/
                => Assoc::RightAssociative,
        }
    }

    pub fn prefix_unary_from_token(token: TokenKind) -> Operator {
        match token {
            TokenKind::Suspend => SuspendOperator,
            TokenKind::Await => AwaitOperator,
            TokenKind::Exclamation => LogicalNotOperator,
            TokenKind::Tilde => NotOperator,
            TokenKind::PlusPlus => PrefixIncrementOperator,
            TokenKind::MinusMinus => PrefixDecrementOperator,
            TokenKind::Dollar => DollarOperator,
            TokenKind::Plus => UnaryPlusOperator,
            TokenKind::Minus => UnaryMinusOperator,
            TokenKind::Ampersand => ReferenceOperator,
            TokenKind::At => ErrorControlOperator,
            TokenKind::New => NewOperator,
            TokenKind::Clone => CloneOperator,
            TokenKind::Include => IncludeOperator,
            TokenKind::Include_once => IncludeOnceOperator,
            TokenKind::Require => RequireOperator,
            TokenKind::Require_once => RequireOnceOperator,
            TokenKind::Print => PrintOperator,
            _ => panic!("not a unary operator"),
        }
    }

    /* Is this a token that can appear after an expression? */
    pub fn is_trailing_operator_token(token: TokenKind) -> bool {
        match token {
            TokenKind::And
            | TokenKind::Or
            | TokenKind::Xor
            | TokenKind::PlusPlus
            | TokenKind::MinusMinus
            | TokenKind::LeftParen
            | TokenKind::LeftBracket
            | TokenKind::LeftBrace
            | TokenKind::Plus
            | TokenKind::Minus
            | TokenKind::Ampersand
            | TokenKind::BarGreaterThan
            | TokenKind::Question
            | TokenKind::QuestionQuestion
            | TokenKind::QuestionQuestionEqual
            | TokenKind::QuestionColon
            | TokenKind::BarBar
            | TokenKind::Carat
            | TokenKind::AmpersandAmpersand
            | TokenKind::Bar
            | TokenKind::EqualEqual
            | TokenKind::EqualEqualEqual
            | TokenKind::LessThanGreaterThan
            | TokenKind::ExclamationEqual
            | TokenKind::ExclamationEqualEqual
            | TokenKind::LessThanEqualGreaterThan
            | TokenKind::LessThan
            | TokenKind::LessThanEqual
            | TokenKind::GreaterThan
            | TokenKind::GreaterThanEqual
            | TokenKind::LessThanLessThan
            | TokenKind::GreaterThanGreaterThan
            | TokenKind::Dot
            | TokenKind::Star
            | TokenKind::Slash
            | TokenKind::Percent
            | TokenKind::Instanceof
            | TokenKind::Is
            | TokenKind::As
            | TokenKind::QuestionAs
            | TokenKind::StarStar
            | TokenKind::Equal
            | TokenKind::PlusEqual
            | TokenKind::MinusEqual
            | TokenKind::StarEqual
            | TokenKind::SlashEqual
            | TokenKind::StarStarEqual
            | TokenKind::DotEqual
            | TokenKind::PercentEqual
            | TokenKind::AmpersandEqual
            | TokenKind::BarEqual
            | TokenKind::CaratEqual
            | TokenKind::LessThanLessThanEqual
            | TokenKind::GreaterThanGreaterThanEqual
            | TokenKind::MinusGreaterThan
            | TokenKind::QuestionMinusGreaterThan
            | TokenKind::ColonColon
            | TokenKind::ColonAt => true,
            _ => false,
        }
    }

    pub fn trailing_from_token(token: TokenKind) -> Operator {
        match token {
            TokenKind::And => PHPAndOperator,
            TokenKind::Or => PHPOrOperator,
            TokenKind::Xor => PHPExclusiveOrOperator,
            TokenKind::BarGreaterThan => PipeOperator,
            TokenKind::Question => ConditionalQuestionOperator,
            TokenKind::Colon => ConditionalColonOperator,
            TokenKind::QuestionQuestion => CoalesceOperator,
            TokenKind::QuestionQuestionEqual => CoalesceAssignmentOperator,
            TokenKind::QuestionColon => DegenerateConditionalOperator,
            TokenKind::BarBar => LogicalOrOperator,
            TokenKind::Carat => ExclusiveOrOperator,
            TokenKind::AmpersandAmpersand => LogicalAndOperator,
            TokenKind::Bar => OrOperator,
            TokenKind::Ampersand => AndOperator,
            TokenKind::EqualEqual => EqualOperator,
            TokenKind::EqualEqualEqual => StrictEqualOperator,
            TokenKind::ExclamationEqual => NotEqualOperator,
            TokenKind::LessThanGreaterThan => PhpNotEqualOperator,
            TokenKind::ExclamationEqualEqual => StrictNotEqualOperator,
            TokenKind::LessThan => LessThanOperator,
            TokenKind::LessThanEqualGreaterThan => SpaceshipOperator,
            TokenKind::LessThanEqual => LessThanOrEqualOperator,
            TokenKind::GreaterThan => GreaterThanOperator,
            TokenKind::GreaterThanEqual => GreaterThanOrEqualOperator,
            TokenKind::LessThanLessThan => LeftShiftOperator,
            TokenKind::GreaterThanGreaterThan => RightShiftOperator,
            TokenKind::Plus => AdditionOperator,
            TokenKind::Minus => SubtractionOperator,
            TokenKind::Dot => ConcatenationOperator,
            TokenKind::Star => MultiplicationOperator,
            TokenKind::Slash => DivisionOperator,
            TokenKind::Percent => RemainderOperator,
            TokenKind::Instanceof => InstanceofOperator,
            TokenKind::Is => IsOperator,
            TokenKind::As => AsOperator,
            TokenKind::QuestionAs => NullableAsOperator,
            TokenKind::StarStar => ExponentOperator,
            TokenKind::Equal => AssignmentOperator,
            TokenKind::PlusEqual => AdditionAssignmentOperator,
            TokenKind::MinusEqual => SubtractionAssignmentOperator,
            TokenKind::StarEqual => MultiplicationAssignmentOperator,
            TokenKind::SlashEqual => DivisionAssignmentOperator,
            TokenKind::StarStarEqual => ExponentiationAssignmentOperator,
            TokenKind::DotEqual => ConcatenationAssignmentOperator,
            TokenKind::PercentEqual => RemainderAssignmentOperator,
            TokenKind::AmpersandEqual => AndAssignmentOperator,
            TokenKind::BarEqual => OrAssignmentOperator,
            TokenKind::CaratEqual => ExclusiveOrAssignmentOperator,
            TokenKind::LessThanLessThanEqual => LeftShiftAssignmentOperator,
            TokenKind::GreaterThanGreaterThanEqual => RightShiftAssignmentOperator,
            TokenKind::MinusGreaterThan => MemberSelectionOperator,
            TokenKind::QuestionMinusGreaterThan => NullSafeMemberSelectionOperator,
            TokenKind::ColonColon => ScopeResolutionOperator,
            TokenKind::PlusPlus => PostfixIncrementOperator,
            TokenKind::MinusMinus => PostfixDecrementOperator,
            TokenKind::LeftParen => FunctionCallOperator,
            TokenKind::LeftBracket => IndexingOperator,
            TokenKind::LeftBrace => IndexingOperator,
            TokenKind::ColonAt => ScopeResolutionOperator,
            _ => panic!("not a trailing operator"),
        }
    }

    pub fn is_binary_operator_token(token: TokenKind) -> bool {
        match token {
            TokenKind::And
            | TokenKind::Or
            | TokenKind::Xor
            | TokenKind::Plus
            | TokenKind::Minus
            | TokenKind::Ampersand
            | TokenKind::BarGreaterThan
            | TokenKind::QuestionQuestion
            | TokenKind::QuestionQuestionEqual
            | TokenKind::QuestionColon
            | TokenKind::BarBar
            | TokenKind::Carat
            | TokenKind::AmpersandAmpersand
            | TokenKind::Bar
            | TokenKind::EqualEqual
            | TokenKind::EqualEqualEqual
            | TokenKind::ExclamationEqual
            | TokenKind::LessThanGreaterThan
            | TokenKind::ExclamationEqualEqual
            | TokenKind::LessThanEqualGreaterThan
            | TokenKind::LessThan
            | TokenKind::LessThanEqual
            | TokenKind::GreaterThan
            | TokenKind::GreaterThanEqual
            | TokenKind::LessThanLessThan
            | TokenKind::GreaterThanGreaterThan
            | TokenKind::Dot
            | TokenKind::Star
            | TokenKind::Slash
            | TokenKind::Percent
            | TokenKind::StarStar
            | TokenKind::Equal
            | TokenKind::PlusEqual
            | TokenKind::MinusEqual
            | TokenKind::StarEqual
            | TokenKind::SlashEqual
            | TokenKind::DotEqual
            | TokenKind::PercentEqual
            | TokenKind::AmpersandEqual
            | TokenKind::BarEqual
            | TokenKind::CaratEqual
            | TokenKind::LessThanLessThanEqual
            | TokenKind::GreaterThanGreaterThanEqual
            | TokenKind::MinusGreaterThan
            | TokenKind::QuestionMinusGreaterThan => true,
            _ => false,
        }
    }

    pub fn is_assignment(&self) -> bool {
        match self {
            AssignmentOperator
            | AdditionAssignmentOperator
            | SubtractionAssignmentOperator
            | MultiplicationAssignmentOperator
            | DivisionAssignmentOperator
            | ExponentiationAssignmentOperator
            | ConcatenationAssignmentOperator
            | RemainderAssignmentOperator
            | AndAssignmentOperator
            | OrAssignmentOperator
            | ExclusiveOrAssignmentOperator
            | LeftShiftAssignmentOperator
            | RightShiftAssignmentOperator
            | CoalesceAssignmentOperator => true,
            _ => false,
        }
    }

    pub fn is_comparison(&self) -> bool {
        match self {
            EqualOperator
            | StrictEqualOperator
            | NotEqualOperator
            | PhpNotEqualOperator
            | StrictNotEqualOperator
            | LessThanOperator
            | LessThanOrEqualOperator
            | GreaterThanOperator
            | GreaterThanOrEqualOperator
            | SpaceshipOperator => true,
            _ => false,
        }
    }
}
