// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

use anyhow::anyhow;
use anyhow::bail;
use anyhow::Result;
use newtype::newtype_int;

// 1-based line number.
newtype_int!(Line, u32, LineMap, LineSet);

#[derive(Debug, PartialEq, Eq, Copy, Clone, strum::IntoStaticStr)]
pub(crate) enum Token<'a> {
    // See Lexer::from_slice for regex definitions
    Global(&'a [u8], Line),
    Variable(&'a [u8], Line),
    TripleStrLiteral(&'a [u8], Line),
    Decl(&'a [u8], Line),
    StrLiteral(&'a [u8], Line),
    Variadic(Line),
    Semicolon(Line),
    Dash(Line),
    OpenCurly(Line),
    OpenBracket(Line),
    OpenParen(Line),
    CloseParen(Line),
    CloseBracket(Line),
    CloseCurly(Line),
    Equal(Line),
    Number(&'a [u8], Line),
    Comma(Line),
    Lt(Line),
    Gt(Line),
    Colon(Line),
    Identifier(&'a [u8], Line),
    Newline(Line),
    Error(&'a [u8], Line),
}

impl<'a> Token<'a> {
    pub(crate) fn error(&self, err: impl std::fmt::Display) -> anyhow::Error {
        anyhow!("Error [line {line}]: {err} ({self:?})", line = self.line())
    }

    pub(crate) fn line(&self) -> Line {
        match self {
            Token::CloseBracket(u)
            | Token::CloseCurly(u)
            | Token::CloseParen(u)
            | Token::Colon(u)
            | Token::Comma(u)
            | Token::Dash(u)
            | Token::Decl(_, u)
            | Token::Equal(u)
            | Token::Error(_, u)
            | Token::Global(_, u)
            | Token::Gt(u)
            | Token::Identifier(_, u)
            | Token::Lt(u)
            | Token::Newline(u)
            | Token::Number(_, u)
            | Token::OpenBracket(u)
            | Token::OpenCurly(u)
            | Token::OpenParen(u)
            | Token::Semicolon(u)
            | Token::StrLiteral(_, u)
            | Token::TripleStrLiteral(_, u)
            | Token::Variable(_, u)
            | Token::Variadic(u) => *u,
        }
    }

    pub(crate) fn as_bytes(&self) -> &'a [u8] {
        match self {
            Token::Global(u, _)
            | Token::Variable(u, _)
            | Token::TripleStrLiteral(u, _)
            | Token::Decl(u, _)
            | Token::StrLiteral(u, _)
            | Token::Number(u, _)
            | Token::Identifier(u, _)
            | Token::Error(u, _) => u,
            Token::Semicolon(_) => b";",
            Token::Dash(_) => b"-",
            Token::OpenCurly(_) => b"{",
            Token::OpenBracket(_) => b"[",
            Token::OpenParen(_) => b"(",
            Token::CloseParen(_) => b")",
            Token::CloseBracket(_) => b"]",
            Token::CloseCurly(_) => b"}",
            Token::Equal(_) => b"=",
            Token::Comma(_) => b",",
            Token::Lt(_) => b"<",
            Token::Gt(_) => b">",
            Token::Colon(_) => b":",
            Token::Variadic(_) => b"...",
            Token::Newline(_) => b"\n",
        }
    }

    pub(crate) fn as_str(&self) -> Result<&str> {
        Ok(std::str::from_utf8(self.as_bytes())?)
    }

    /// Only str_literal and triple_str_literal can be parsed into a new tokenizer.
    /// To create a new tokenizer that still has accurate error reporting, we want to pass the line
    /// So `into_str_literal_and_line` and `into_triple_str_literal_and_line` return a Result of bytes rep and line # or bail
    pub(crate) fn into_triple_str_literal_and_line(self) -> Result<(&'a [u8], Line)> {
        match self {
            Token::TripleStrLiteral(vec_u8, pos) => Ok((vec_u8, pos)),
            _ => bail!("Expected a triple str literal, got: {}", self),
        }
    }

    pub(crate) fn into_global(self) -> Result<&'a [u8]> {
        match self {
            Token::Global(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a global, got: {}", self),
        }
    }

    pub(crate) fn into_variable(self) -> Result<&'a [u8]> {
        match self {
            Token::Variable(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a variable, got: {}", self),
        }
    }

    pub(crate) fn into_triple_str_literal(self) -> Result<&'a [u8]> {
        match self {
            Token::TripleStrLiteral(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a triple str literal, got: {}", self),
        }
    }

    #[cfg(test)]
    pub(crate) fn into_decl(self) -> Result<&'a [u8]> {
        match self {
            Token::Decl(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a decl, got: {}", self),
        }
    }

    pub(crate) fn into_str_literal(self) -> Result<&'a [u8]> {
        match self {
            Token::StrLiteral(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a str literal, got: {}", self),
        }
    }

    pub(crate) fn into_unquoted_str_literal(self) -> Result<&'a [u8]> {
        Ok(escaper::unquote_slice(self.into_str_literal()?))
    }

    pub(crate) fn into_number(self) -> Result<&'a [u8]> {
        match self {
            Token::Number(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected a number, got: {}", self),
        }
    }

    pub(crate) fn into_identifier(self) -> Result<&'a [u8]> {
        match self {
            Token::Identifier(vec_u8, _) => Ok(vec_u8),
            _ => bail!("Expected an identifier, got: {}", self),
        }
    }

    pub(crate) fn into_dash(self) -> Result<&'a [u8]> {
        match self {
            Token::Dash(_) => Ok(self.as_bytes()),
            _ => bail!("Expected a dash, got: {}", self),
        }
    }

    pub(crate) fn is_triple_str_literal(&self) -> bool {
        matches!(self, Token::TripleStrLiteral(..))
    }

    pub(crate) fn is_decl(&self) -> bool {
        matches!(self, Token::Decl(..))
    }

    pub(crate) fn is_str_literal(&self) -> bool {
        matches!(self, Token::StrLiteral(..))
    }

    pub(crate) fn is_number(&self) -> bool {
        matches!(self, Token::Number(..))
    }

    pub(crate) fn is_identifier(&self) -> bool {
        matches!(self, Token::Identifier(..))
    }

    pub(crate) fn is_comma(&self) -> bool {
        matches!(self, Token::Comma(_))
    }

    pub(crate) fn is_semicolon(&self) -> bool {
        matches!(self, Token::Semicolon(_))
    }

    pub(crate) fn is_colon(&self) -> bool {
        matches!(self, Token::Colon(_))
    }

    pub(crate) fn is_dash(&self) -> bool {
        matches!(self, Token::Dash(_))
    }

    pub(crate) fn is_open_bracket(&self) -> bool {
        matches!(self, Token::OpenBracket(_))
    }

    pub(crate) fn is_open_paren(&self) -> bool {
        matches!(self, Token::OpenParen(_))
    }

    pub(crate) fn is_close(&self) -> bool {
        matches!(
            self,
            Token::CloseParen(_) | Token::CloseBracket(_) | Token::CloseCurly(_)
        )
    }

    pub(crate) fn is_close_paren(&self) -> bool {
        matches!(self, Token::CloseParen(_))
    }

    pub(crate) fn is_close_bracket(&self) -> bool {
        matches!(self, Token::CloseBracket(_))
    }

    pub(crate) fn is_open_curly(&self) -> bool {
        matches!(self, Token::OpenCurly(_))
    }

    pub(crate) fn is_close_curly(&self) -> bool {
        matches!(self, Token::CloseCurly(_))
    }

    pub(crate) fn is_equal(&self) -> bool {
        matches!(self, Token::Equal(_))
    }

    pub(crate) fn is_lt(&self) -> bool {
        matches!(self, Token::Lt(_))
    }

    pub(crate) fn is_gt(&self) -> bool {
        matches!(self, Token::Gt(_))
    }

    pub(crate) fn is_variadic(&self) -> bool {
        matches!(self, Token::Variadic(_))
    }
}

impl fmt::Display for Token<'_> {
    /// Purpose of this fmt: so that vec of u8 (internal str representation of each token) is printed as a string rather than bytes
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let text = std::str::from_utf8(self.as_bytes()).map_err(|_| fmt::Error)?;
        let variant: &str = (*self).into();
        let line = self.line();
        write!(f, r#"{variant}("{text}", line: {line})"#)
    }
}
