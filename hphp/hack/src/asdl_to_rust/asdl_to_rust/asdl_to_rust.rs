// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// e.g. cd ~/fbsource && buck2 run fbcode//hphp/hack/src/asdl_to_rust:asdl_to_rust -- --rustfmt-path $(which rustfmt) fbcode/hphp/hack/src/asdl_to_rust/python/v3.10.0.asdl

use std::io::Read;
use std::io::Write;

use convert_case::Case;
use convert_case::Casing;

/// Generate Rust datatypes for abstract syntax defined by ASDL.
#[derive(Debug, clap::Parser)]
struct Opts {
    src: std::path::PathBuf,

    #[clap(short, long)]
    rustfmt_path: std::path::PathBuf,
}

fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync + 'static>> {
    let opts = parse_opts()?;

    let lexerdef = asdl::lexer::lexerdef();
    let asdl = std::fs::read_to_string(&opts.src)?;
    let lexer = lexerdef.lexer(&asdl);
    let (res, errs) = asdl::parser::parse(&lexer);
    for e in errs {
        println!("{}", e.pp(&lexer, &asdl::parser::token_epp));
    }

    let mut out_file = tempfile::NamedTempFile::new()?;
    match res {
        Some(modu) => {
            //println!("Result: {:#?}", r),
            types_of_asdl(&modu.ok().unwrap(), &mut out_file)?;

            let _status = std::process::Command::new(&opts.rustfmt_path)
                .arg(out_file.path())
                .status()
                .expect("failed to execute rustfmt");

            let mut in_file = out_file.reopen()?;
            let mut buf = String::new();
            in_file.read_to_string(&mut buf)?;
            std::io::stdout().write_all(buf.as_bytes())?;
        }
        _ => eprintln!("Unable to evaluate expression."),
    }
    Ok(())
}

fn parse_opts() -> Result<Opts, std::io::Error> {
    let opts = <Opts as clap::Parser>::parse();
    let src = &opts.src;
    std::fs::metadata(src)?;
    Ok(opts)
}

fn types_of_asdl(
    modu: &asdl::parser::ast::Module,
    w: &mut dyn std::io::Write,
) -> std::io::Result<()> {
    for def in &modu.definitions {
        type_declaration_of_definition(def, w)?;
    }
    Ok(())
}

fn type_declaration_of_definition(
    def: &asdl::parser::ast::Definition,
    w: &mut dyn std::io::Write,
) -> std::io::Result<()> {
    let s: &str = &def.type_id;
    let typename: String = make_valid_type_name(s);
    match &def.desc {
        asdl::parser::ast::Desc::Product(asdl::parser::ast::Product(fields)) => {
            w.write_all(TYPE_HERALD)?;
            write!(w, "pub struct {} {{", typename)?;
            for f in fields {
                label_declaration_of_field(f, w)?;
            }
            write!(w, "}}\n")?;
        }
        asdl::parser::ast::Desc::Sum(_) => {}
    };

    Ok(())
}

fn label_declaration_of_field(
    field: &asdl::parser::ast::Field,
    w: &mut dyn std::io::Write,
) -> std::io::Result<()> {
    let id = make_valid_label(&field.id);
    let typ_id = type_of_field(field);
    match field.modifier {
        Some(ref u) => match u {
            asdl::parser::ast::Modifier::Star => write!(w, "pub {id}: Vec<{typ_id}>,"),
            asdl::parser::ast::Modifier::Question => write!(w, "pub {id}: Option<{typ_id}>,"),
        },
        None => write!(w, "pub {id}: {typ_id},\n"),
    }?;
    Ok(())
}

fn type_of_field(field: &asdl::parser::ast::Field) -> String {
    match kind_of_field(field) {
        Kind::String => "String".to_owned(),
        Kind::Int => "isize".to_owned(),
        Kind::Bool => "bool".to_owned(),
        Kind::Type(s) => s,
    }
}

enum Kind {
    String,
    Bool,
    Int,
    Type(String),
}

fn kind_of_field(field: &asdl::parser::ast::Field) -> Kind {
    match field.type_id.as_str() {
        "string" | "bytes" | "identifier" => Kind::String,
        "int" => match field.id.as_str() {
            "is_async" => Kind::Bool,
            _ => Kind::Int,
        },
        "bool" => Kind::Bool,
        _ => Kind::Type(make_valid_type_name(&field.type_id)),
    }
}

// ---

fn make_valid_label(s: &str) -> String {
    let t = s.to_case(Case::Snake);
    match t.as_str() {
        s if RUST_KEYWORDS.contains(&s) => s.to_owned() + "_",
        _ => t,
    }
}

fn make_valid_type_name(s: &str) -> String {
    s.to_case(Case::Pascal)
}

static TYPE_HERALD: &[u8] = b"
#[derive(Clone, Debug, PartialEq, PartialOrd)]\n
#[derive(Serialize, Deserialize, ToOcamlRep, FromOcamlRep)]\n
#[rust_to_ocaml(and)]";

static RUST_KEYWORDS: &[&str] = &[
    "as", "break", "const", "continue", "crate", "else", "enum", "extern", "false", "fn", "for",
    "if", "impl", "in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return",
    "self", "Self", "static", "struct", "super", "trait", "true", "type", "unsafe", "use", "where",
    "while", "dyn", "abstract", "become", "box", "do", "final", "macro", "override", "priv",
    "typeof", "unsized", "virtual", "yield", "async", "await", "try", "union",
];
