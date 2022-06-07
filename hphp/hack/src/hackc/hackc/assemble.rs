// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::FileOpts;
use anyhow::{anyhow, Result};
use bumpalo::Bump;
use clap::Parser;
use hhbc::hackc_unit::HackCUnit;
use options::Options;
use oxidized::relative_path::{self, RelativePath};
use rayon::prelude::*;
use regex::bytes::Regex;

use std::{
    fmt,
    fs::{self, File},
    io::{stdout, Write},
    path::{Path, PathBuf},
    sync::Mutex,
};

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    /// The input hhas file(s) to deserialize back to HackCUnit
    #[clap(flatten)]
    files: FileOpts,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

#[derive(Debug, PartialEq, Eq, Copy, Clone)]
pub struct Pos {
    pub line: usize,
    pub col: usize,
}

#[derive(Debug, PartialEq, Eq)]
pub enum Token<'a> {
    //see below in Lexer::new for regex definitions
    Global(&'a [u8], Pos),
    Variable(&'a [u8], Pos),
    TripleStrLiteral(&'a [u8], Pos),
    Comment(&'a [u8], Pos),
    Decl(&'a [u8], Pos),
    StrLiteral(&'a [u8], Pos),
    Variadic(Pos),
    Semicolon(Pos),
    Dash(Pos),
    OpenCurly(Pos),
    OpenBracket(Pos),
    OpenParen(Pos),
    CloseParen(Pos),
    CloseBracket(Pos),
    CloseCurly(Pos),
    Equal(Pos),
    Number(&'a [u8], Pos),
    Comma(Pos),
    Lt(Pos),
    Gt(Pos),
    Colon(Pos),
    Identifier(&'a [u8], Pos),
    Error(&'a [u8], Pos),
}
impl<'a> Token<'a> {
    fn as_bytes(&self) -> &'a [u8] {
        match self {
            Token::Global(u, _)
            | Token::Variable(u, _)
            | Token::TripleStrLiteral(u, _)
            | Token::Comment(u, _)
            | Token::Decl(u, _)
            | Token::StrLiteral(u, _)
            | Token::Number(u, _)
            | Token::Identifier(u, _)
            | Token::Error(u, _) => u,
            Token::Semicolon(_) => ";".as_bytes(),
            Token::Dash(_) => "-".as_bytes(),
            Token::OpenCurly(_) => "{".as_bytes(),
            Token::OpenBracket(_) => "[".as_bytes(),
            Token::OpenParen(_) => "(".as_bytes(),
            Token::CloseParen(_) => ")".as_bytes(),
            Token::CloseBracket(_) => "]".as_bytes(),
            Token::CloseCurly(_) => "}".as_bytes(),
            Token::Equal(_) => "=".as_bytes(),
            Token::Comma(_) => ",".as_bytes(),
            Token::Lt(_) => "<".as_bytes(),
            Token::Gt(_) => ">".as_bytes(),
            Token::Colon(_) => ":".as_bytes(),
            Token::Variadic(_) => "...".as_bytes(),
        }
    }
}
impl<'a> fmt::Display for Token<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let text = std::str::from_utf8(self.as_bytes()).map_err(|_| fmt::Error)?;
        match self {
            Token::Global(_, pos) => write!(f, "Global(\"{text}\", {pos:?})"),
            Token::Variable(_, pos) => write!(f, "Variable(\"{text}\", {pos:?})"),
            Token::TripleStrLiteral(_, pos) => write!(f, "TripleStrLiteral(\"{text}\", {pos:?})"),
            Token::Comment(_, pos) => write!(f, "Comment(\"{text}\", {pos:?})"),
            Token::Decl(_, pos) => write!(f, "Decl(\"{text}\", {pos:?})"),
            Token::StrLiteral(_, pos) => write!(f, "StrLiteral(\"{text}\", {pos:?})"),
            Token::Number(_, pos) => write!(f, "Number(\"{text}\", {pos:?})"),
            Token::Identifier(_, pos) => write!(f, "Identifier(\"{text}\", {pos:?})"),
            Token::Error(_, pos) => write!(f, "Error(\"{text}\", {pos:?})"),
            Token::Semicolon(pos) => write!(f, "Semicolon(\"{text}\", {pos:?})"),
            Token::Dash(pos) => write!(f, "Dash(\"{text}\", {pos:?})"),
            Token::OpenCurly(pos) => write!(f, "OpenCurly(\"{text}\", {pos:?})"),
            Token::OpenBracket(pos) => write!(f, "OpenBracket(\"{text}\", {pos:?})"),
            Token::OpenParen(pos) => write!(f, "OpenParen(\"{text}\", {pos:?})"),
            Token::CloseParen(pos) => write!(f, "CloseParen(\"{text}\", {pos:?})"),
            Token::CloseBracket(pos) => write!(f, "CloseBracket(\"{text}\", {pos:?})"),
            Token::CloseCurly(pos) => write!(f, "CloseCurly(\"{text}\", {pos:?})"),
            Token::Equal(pos) => write!(f, "Equal(\"{text}\", {pos:?})"),
            Token::Comma(pos) => write!(f, "Comma(\"{text}\", {pos:?})"),
            Token::Lt(pos) => write!(f, "Lt(\"{text}\", {pos:?})"),
            Token::Gt(pos) => write!(f, "Gt(\"{text}\", {pos:?})"),
            Token::Colon(pos) => write!(f, "Colon(\"{text}\", {pos:?})"),
            Token::Variadic(pos) => write!(f, "Variadic(\"{text}\", {pos:?})"),
        }
    }
}
// We initially planned on using Logos, a crate for tokenizing really fast.
// We chose not to use Logos because it doesn't support all regexes -- for instance, it can't
// tokenize based on the regex "\"\"\".*\"\"\"". Here's the git issue:
// https://github.com/maciejhirsz/logos/issues/246
pub struct Lexer<'a> {
    tokens: Vec<Token<'a>>,
}

impl<'a> IntoIterator for Lexer<'a> {
    type Item = Token<'a>;
    type IntoIter = <Vec<Token<'a>> as IntoIterator>::IntoIter;

    fn into_iter(self) -> Self::IntoIter {
        self.tokens.into_iter()
    }
}
fn build_tokens_helper<'a>(
    s: &'a [u8],
    cur_pos: &mut Pos,
    tokens: &mut Vec<Token<'a>>,
    big_regex: &Regex,
) -> &'a [u8] {
    if let Some(mat) = big_regex.find(s) {
        let mut chars = s.iter(); //implicit assumption: matched to the start (^), so we iter from the start
        debug_assert!(mat.start() == 0);
        match chars.next().unwrap() {
            //get first character
            b'\n' => {
                cur_pos.line += 1;
                cur_pos.col = 1;
                &s[mat.end()..]
            }
            //Note these don't match what prints out on a printer, but not sure how to generalize
            b'\x0C' => {
                //form feed
                cur_pos.col += 1;
                &s[mat.end()..]
            }
            b'\r' => {
                cur_pos.col = 1;
                &s[mat.end()..]
            }
            b'\t' => {
                cur_pos.col += 4;
                &s[mat.end()..]
            }
            b' ' => {
                cur_pos.col += 1;
                &s[mat.end()..]
            } //don't add whitespace as tokens, just increase line and col
            o => {
                let end = mat.end();
                let tok = match o {
                    b'#' => Token::Comment(&s[..end], *cur_pos),  //comment
                    b'@' => Token::Global(&s[..end], *cur_pos),   //global
                    b'$' => Token::Variable(&s[..end], *cur_pos), //var
                    b'.' => {
                        if *(chars.next().unwrap()) == b'.' && *(chars.next().unwrap()) == b'.' {
                            //variadic
                            Token::Variadic(*cur_pos)
                        } else {
                            Token::Decl(&s[..end], *cur_pos) //decl
                        }
                    }
                    b';' => Token::Semicolon(*cur_pos), //semicolon
                    b'{' => Token::OpenCurly(*cur_pos), //opencurly
                    b'[' => Token::OpenBracket(*cur_pos),
                    b'(' => Token::OpenParen(*cur_pos),
                    b')' => Token::CloseParen(*cur_pos),
                    b']' => Token::CloseBracket(*cur_pos),
                    b'}' => Token::CloseCurly(*cur_pos),
                    b',' => Token::Comma(*cur_pos),
                    b'<' => Token::Lt(*cur_pos),    //<
                    b'>' => Token::Gt(*cur_pos),    //>
                    b'=' => Token::Equal(*cur_pos), //=
                    b'-' => {
                        if chars.next().unwrap().is_ascii_digit() {
                            //negative number
                            Token::Number(&s[..end], *cur_pos)
                        } else {
                            Token::Dash(*cur_pos)
                        }
                    }
                    b':' => Token::Colon(*cur_pos),
                    b'"' => {
                        if *(chars.next().unwrap()) == b'"' && *(chars.next().unwrap()) == b'"' {
                            //triple string literal
                            Token::TripleStrLiteral(&s[..end], *cur_pos)
                        } else {
                            //single string literal
                            Token::StrLiteral(&s[..end], *cur_pos)
                        }
                    }
                    dig_or_id => {
                        if dig_or_id.is_ascii_digit()
                            || (*dig_or_id as char == '+'
                                && (chars.next().unwrap()).is_ascii_digit())
                        //positive numbers denoted with +
                        {
                            Token::Number(&s[..end], *cur_pos)
                        } else {
                            Token::Identifier(&s[..end], *cur_pos)
                        }
                    }
                };
                tokens.push(tok);
                cur_pos.col += end - mat.start(); //advance col by length of token
                &s[end..]
            }
        }
    } else {
        //couldn't tokenize the string, so add the rest of it as an error
        tokens.push(Token::Error(
            s,
            Pos {
                line: cur_pos.line,
                col: cur_pos.col,
            },
        ));
        //done advancing col and line cuz at end
        &[]
    }
}
impl<'a> Lexer<'a> {
    pub fn from_str(s: &'a [u8]) -> Self {
        //first create the regex that matches any token. Done this way for readability
        let v = [
            r#"""".*""""#,                                          //triple str literal
            "#.*",                                                  //comment
            r"(?-u)[\.@][_a-zA-Z\x80-\xff][_/a-zA-Z0-9\x80-\xff]*", //decl, var, global. (?-u) turns off utf8 check
            r"(?-u)\$[_a-zA-Z0-9\x80-\xff][_/a-zA-Z0-9\x80-\xff]*", //var. See /home/almathaler/fbsource/fbcode/hphp/test/quick/reified-and-variadic.php's assembly for a var w/ a digit at front
            r#""((\\.)|[^\\"])*""#,                                 //str literal
            r"[-+]?[0-9]+\.?[0-9]*",                                //number
            r"(?-u)[_/a-zA-Z\x80-\xff][_/\\a-zA-Z0-9\x80-\xff]*",   //identifier
            ";",
            "-",
            "=",
            r"\{",
            r"\[",
            r"\(",
            r"\)",
            r"\]",
            r"\}",
            ",",
            "<",
            ">",
            ":",
            r"\.\.\.", //variadic
            "\n",
            r"[ \t\r\f]+",
        ];
        let big_regex = format!("^(({}))", v.join(")|("));
        let big_regex = Regex::new(&big_regex).unwrap();
        let mut cur_pos = Pos { line: 1, col: 1 };
        let mut tokens = Vec::new();
        let mut source = s;
        while !source.is_empty() {
            source = build_tokens_helper(source, &mut cur_pos, &mut tokens, &big_regex);
        }
        Lexer { tokens }
    }
}

/// Tokenizes input string using a Lexer. Prints all tokens in the Lexer
fn print_tokens<'a>(s: &'a [u8]) {
    let lex: Lexer<'a> = Lexer::from_str(s);
    for tok in lex {
        println!("{}", tok);
    }
}
/// Assembles the hhas represented by the vec of bytes input
fn assemble_from_bytes<'arena>(
    _alloc: &'arena Bump,
    s: &[u8],
    _opts: &Opts,
) -> Result<HackCUnit<'arena>> {
    //lifetime of lexer is tied to lifetime of bytes
    print_tokens(s);
    let tr: Result<HackCUnit<'_>> = Ok(Default::default());
    tr
}

/// Assembles the hhas within f to a HackCUnit. Currently just returns default HCU
pub fn assemble<'arena>(alloc: &'arena Bump, f: &Path, opts: &Opts) -> Result<HackCUnit<'arena>> {
    let s: Vec<u8> = fs::read(f)?;
    let s = s.as_slice();
    let _tr: Result<HackCUnit<'_>> = assemble_from_bytes(alloc, s, opts);
    todo!()
}

pub fn run(mut opts: Opts) -> Result<()> {
    //Create writer to output/stdout
    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };
    //May have specified multiple files
    let files = opts.files.gather_input_files()?;
    //Process each file
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &opts, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Assemble the hhas in a given file to a HackCUnit. Then use bytecode printer
/// to write the hhas representation of that HCU to output
/// 5/31: Side-effect: prints tokenized input file to terminal
pub fn process_one_file(f: &Path, opts: &Opts, w: &SyncWrite) -> Result<()> {
    let alloc = Bump::default();
    //if it's not an hhas file don't assemble. Return Err(e):
    if Path::extension(f) == Some(std::ffi::OsStr::new("hhas")) {
        let hcu = assemble(&alloc, f, opts)?; //assemble will print the tokens to output
        let filepath = RelativePath::make(relative_path::Prefix::Dummy, f.to_owned());
        let comp_options: Options = Default::default();
        //note: why not make a native_env based on the filepath
        //and then use its to_options -- why is to_options() private?
        let ctxt = bytecode_printer::Context::new(&comp_options, Some(&filepath), false);
        let mut output = Vec::new();
        match bytecode_printer::print_unit(&ctxt, &mut output, &hcu) {
            Err(e) => {
                eprintln!("Error bytecode_printing file {}: {}", f.display(), e);
                Err(anyhow!("bytecode_printer problem"))
            }
            Ok(_) => {
                w.lock().unwrap().write_all(&output)?;
                Ok(())
            }
        }
    } else {
        Err(anyhow!(
            "can't assemble non-hhas file: {:?}, extension: {:?}",
            f,
            Path::extension(f).unwrap()
        ))
    }
}

#[cfg(test)]
mod test {
    #[test]
    fn difficult_strings() {
        use crate::assemble::Lexer;
        use crate::assemble::Token;
        let s = r#""\"0\""
        "12345\\:2\\"
        "class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class";
        "#;
        let s = s.as_bytes();
        let l: Lexer<'_> = Lexer::from_str(s);
        let mut l = l.into_iter();
        //expecting 3 string tokens
        let _st1 = l.next().unwrap();
        let _by1 = str::as_bytes(r#""\"0\"""#);
        assert!(matches!(_st1, Token::StrLiteral(_by1, _)));
        let _st2 = l.next().unwrap();
        let _by2 = str::as_bytes(r#""12345\\:2\\""#);
        assert!(matches!(_st1, Token::StrLiteral(_by2, _)));
        let _st3 = l.next().unwrap();
        let _by3 = str::as_bytes(
            r#""class_meth() expects a literal class name or ::class constant, followed by a constant string that refers to a static method on that class""#,
        );
        assert!(matches!(_st1, Token::StrLiteral(_by3, _)));
    }
    #[test]
    fn odd_unicode_test() {
        use crate::assemble::Lexer;
        use crate::assemble::Token;
        let s: &[u8] = b".\xA9\xEF\xB8\x8E $0\xC5\xA3\xB1\xC3 \xE2\x98\xBA\xE2\x98\xBA\xE2\x98\xBA @\xE2\x99\xA1\xE2\x99\xA4$";
        let l: Lexer<'_> = Lexer::from_str(s);
        //we are expecting an decl, a var, an identifier a global, and an error on the last empty variable
        let mut l = l.into_iter();
        let decl = l.next().unwrap();
        assert!(matches!(decl, Token::Decl(..)));
        let var = l.next().unwrap();
        assert!(matches!(var, Token::Variable(..)));
        let iden = l.next().unwrap();
        assert!(matches!(iden, Token::Identifier(..)));
        let glob = l.next().unwrap();
        assert!(matches!(glob, Token::Global(..)));
        let err = l.next().unwrap();
        assert!(matches!(err, Token::Error(..)))
    }

    #[test]
    fn every_token_test() {
        use crate::assemble::Lexer;
        use crate::assemble::Token;
        let s = r#"@_global $0Var """tripleStrLiteral:)""" #hashtagComment
        .Decl "str!Literal" ...
        ;-{[( )]} =98 -98 +101. 43.2 , < > : _/identifier/ /filepath ."#;
        //expect glob var tsl comment decl strlit semicolon dash open_curly open_brack open_paren close_paren close_bracket
        //close_curly equal number number number number , < > : identifier identifier ERROR on the last .
        let s = s.as_bytes();
        let l: Lexer<'_> = Lexer::from_str(s);
        let mut l = l.into_iter();
        let glob = l.next().unwrap();
        eprintln!("{}", glob);
        assert!(matches!(glob, Token::Global(..)));
        let var = l.next().unwrap();
        eprintln!("{}", var);
        assert!(matches!(var, Token::Variable(..)));
        let tsl = l.next().unwrap();
        eprintln!("{}", tsl);
        assert!(matches!(tsl, Token::TripleStrLiteral(..)));
        let comment = l.next().unwrap();
        eprintln!("{}", comment);
        assert!(matches!(comment, Token::Comment(..)));
        let decl = l.next().unwrap();
        eprintln!("{}", decl);
        assert!(matches!(decl, Token::Decl(..)));
        let strlit = l.next().unwrap();
        eprintln!("{}", strlit);
        assert!(matches!(strlit, Token::StrLiteral(..)));
        let variadic = l.next().unwrap();
        eprintln!("{}", variadic);
        assert!(matches!(variadic, Token::Variadic(..)));
        let semicolon = l.next().unwrap();
        eprintln!("{}", semicolon);
        assert!(matches!(semicolon, Token::Semicolon(..)));
        let dash = l.next().unwrap();
        eprintln!("{}", dash);
        assert!(matches!(dash, Token::Dash(..)));
        let oc = l.next().unwrap();
        eprintln!("{}", dash);
        assert!(matches!(oc, Token::OpenCurly(..)));
        let ob = l.next().unwrap();
        eprintln!("{}", ob);
        assert!(matches!(ob, Token::OpenBracket(..)));
        let op = l.next().unwrap();
        eprintln!("{}", op);
        assert!(matches!(op, Token::OpenParen(..)));
        let cp = l.next().unwrap();
        eprintln!("{}", cp);
        assert!(matches!(cp, Token::CloseParen(..)));
        let cb = l.next().unwrap();
        eprintln!("{}", cb);
        assert!(matches!(cb, Token::CloseBracket(..)));
        let cc = l.next().unwrap();
        eprintln!("{}", cc);
        assert!(matches!(cc, Token::CloseCurly(..)));
        let eq = l.next().unwrap();
        eprintln!("{}", eq);
        assert!(matches!(eq, Token::Equal(..)));
        let num = l.next().unwrap();
        eprintln!("{}", num);
        assert!(matches!(num, Token::Number(..)));
        let num = l.next().unwrap();
        eprintln!("{}", num);
        assert!(matches!(num, Token::Number(..)));
        let num = l.next().unwrap();
        eprintln!("{}", num);
        assert!(matches!(num, Token::Number(..)));
        let num = l.next().unwrap();
        eprintln!("{}", num);
        assert!(matches!(num, Token::Number(..)));
        let comma = l.next().unwrap();
        eprintln!("{}", comma);
        assert!(matches!(comma, Token::Comma(..)));
        let lt = l.next().unwrap();
        eprintln!("{}", lt);
        assert!(matches!(lt, Token::Lt(..)));
        let gt = l.next().unwrap();
        eprintln!("{}", gt);
        assert!(matches!(gt, Token::Gt(..)));
        let colon = l.next().unwrap();
        eprintln!("{}", colon);
        assert!(matches!(colon, Token::Colon(..)));
        let iden = l.next().unwrap();
        eprintln!("{}", iden);
        assert!(matches!(iden, Token::Identifier(..)));
        let iden = l.next().unwrap();
        eprintln!("{}", iden);
        assert!(matches!(iden, Token::Identifier(..)));
        let err = l.next().unwrap();
        eprintln!("{}", err);
        assert!(matches!(err, Token::Error(..)));
    }
}
