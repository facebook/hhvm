use std::collections::HashMap;
use std::ffi::OsString;
use std::io::ErrorKind;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;
use clap::Parser;
use clap::Subcommand;
use pulldown_cmark::CodeBlockKind;
use pulldown_cmark::Event;
use pulldown_cmark::Options;
use pulldown_cmark::Parser as MarkdownParser;
use pulldown_cmark::Tag;

/// Handles code samples in markdown files from the Hack manual.
///
/// Quickstart:
///
/// $ cd ~/fbsource/fbcode
/// $ buck run //hphp/hack/src/hh_manual:hh_manual extract hphp/hack/manual/hack/
#[derive(Parser, Debug)]
#[clap(verbatim_doc_comment)]
struct Cli {
    #[clap(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    /// Extract Hack code samples as standalone files suitable for hh_single_type_check.
    ///
    /// For every markdown file in the directory (recursively), extract any example of the
    /// form:
    ///
    /// ```hack
    /// your_sample_code_here();
    /// ```
    ///
    /// Additionally, the following syntax is recognized:
    ///
    /// ```hack no-extract
    /// // This is highlighted as Hack in the manual, but
    /// // not extracted as a standalone file for testing.
    /// ```
    ///
    /// ```hack error
    /// // This is extracted as a .hack_error file, and tests will ensure
    /// // that it actually produces an error.
    /// ```
    ///
    /// ```hack file:foo.hack
    /// // This is extracted as a file named foo.hack. Multiple code blocks
    /// // can use file:foo.hack, and the result will be concatenated.
    /// ```
    ///
    /// Each code block is wrapped in a toplevel function. If the block contains any
    /// toplevel definitions (function, classes, etc), the block is extracted unchanged.
    ///
    /// ```hack
    /// function foo(): void {
    ///   // This code block is extracted as-is.
    /// }
    /// ```
    /// Any previous *.hack, *.hack_error or *.php files are deleted from the destination
    /// directory.
    ///
    /// Other files are untouched, so you can create HH_FLAGS files for any chapters in
    /// the manual.
    #[clap(verbatim_doc_comment)]
    Extract {
        /// The directory containing the chapters of the Hack manual.
        path: PathBuf,
    },
}

#[derive(Debug, Clone)]
struct CodeBlock {
    filename: Option<String>,
    content: String,
    error: bool,
}

/// Does `src` look like toplevel code, such as a class or function
/// definition? If not, it's just a snippet.
fn looks_like_toplevel_code(src: &str) -> bool {
    let toplevel_prefixes = [
        // Functions can start with these.
        "function",
        "async",
        // Classish types.
        "class",
        "trait",
        "interface",
        "enum",
        "abstract",
        "final",
        // Type aliases
        "type",
        "newtype",
        // Constants
        "const",
        // Modules
        "module",
        "new module",
        "internal",
        "public",
        // Using types/namespaces.
        "namespace",
        "use",
    ];

    src.lines().any(|line| {
        for prefix in toplevel_prefixes {
            if line.starts_with(&format!("{} ", prefix)) {
                return true;
            }
        }
        false
    })
}

/// Wrap snippet `src` in a function definition, so it's a valid Hack
/// program.
///
/// If `i` is provided, append it to the function name, so we can have
/// multiple functions in the same file.
fn wrap_snippet(src: &str, i: Option<usize>) -> String {
    let mut res = String::new();
    res.push_str(&format!(
        "async function example_snippet_wrapper{}(): Awaitable<void> {{\n",
        match i {
            Some(i) => format!("{}", i),
            None => "".to_owned(),
        },
    ));

    let can_indent = !src.contains("<<<");
    for line in src.lines() {
        res.push_str(&format!("{}{}\n", if can_indent { "  " } else { "" }, line));
    }

    res.push_str("}\n");
    res
}

/// Given markdown text `src`, extract all the triple-backtick code
/// blocks that are marked as `hack`.
fn extract_hack_blocks(src: &str) -> Result<Vec<CodeBlock>> {
    let options = Options::empty();
    let parser = MarkdownParser::new_ext(src, options);

    let mut res = vec![];
    let mut block_info: Option<String> = None;
    let mut file_snippet_count: HashMap<String, usize> = HashMap::new();

    for event in parser {
        match event {
            Event::Start(Tag::CodeBlock(CodeBlockKind::Fenced(info))) => {
                block_info = Some(info.into_string());
            }
            Event::Text(t) => {
                if let Some(info) = &block_info {
                    if let Some(hack_info) = info.to_lowercase().trim().strip_prefix("hack") {
                        let mut filename = None;
                        let mut should_extract = true;
                        let mut error = false;

                        for part in hack_info.trim().split(' ') {
                            if part.is_empty() {
                                // No metadata after the triple backticks.
                            } else if part == "no-extract" {
                                // Highlighted as Hack, but not extracted
                                should_extract = false;
                                break;
                            } else if part == "error" {
                                error = true;
                            } else if part.starts_with("file:") {
                                filename = Some(part.trim_start_matches("file:").to_owned());
                            } else {
                                return Err(anyhow::anyhow!(
                                    "Invalid code block metadata '{}'",
                                    part,
                                ));
                            }
                        }

                        let content = if looks_like_toplevel_code(&t) {
                            t.into_string()
                        } else {
                            let snippet_i = if let Some(filename) = &filename {
                                *file_snippet_count.entry(filename.to_string()).or_insert(0) += 1;
                                Some(file_snippet_count[filename])
                            } else {
                                None
                            };

                            wrap_snippet(&t, snippet_i)
                        };

                        if should_extract {
                            res.push(CodeBlock {
                                filename,
                                content,
                                error,
                            });
                        }
                    }
                }
            }
            Event::End(_) => {
                block_info = None;
            }
            _ => {}
        }
    }

    Ok(res)
}

/// Concatenate all code blocks with the same `file:foo.hack` filename
/// into a single block.
fn merge_by_filename(code_blocks: &[CodeBlock]) -> Vec<CodeBlock> {
    let mut res: Vec<CodeBlock> = vec![];
    let mut named_blocks: HashMap<String, CodeBlock> = HashMap::new();

    for code_block in code_blocks {
        if let Some(filename) = &code_block.filename {
            named_blocks
                .entry(filename.to_owned())
                .and_modify(|b| b.content = format!("{}\n{}", b.content, code_block.content))
                .or_insert_with(|| code_block.clone());
        } else {
            res.push(code_block.clone());
        }
    }

    res.extend(named_blocks.values().cloned());
    res
}

/// Write `content` as an extracted file to `out_path`.
fn write_example(out_path: &Path, content: &str, page_rel_path: &Path) -> Result<()> {
    let mut out_f = std::fs::File::create(out_path)?;

    let content_lines: Vec<_> = content.lines().collect();

    // Generally we want the first line to be `// generated`. However, hh_single_type_check
    // requires the `//// multifile.hack` comment to be the first line.
    //
    // If we have a multiline comment or shebang, put `// generated` on the second line.
    let first_line_multifile = if let Some(line) = content_lines.first() {
        line.starts_with("////") || line.starts_with("#!")
    } else {
        false
    };

    for (i, line) in content_lines.iter().enumerate() {
        if (i == 0 && !first_line_multifile) || (i == 1 && first_line_multifile) {
            write!(out_f, "// @")?;
            writeln!(
                out_f,
                "generated by hh_manual from {}",
                page_rel_path.display()
            )?;
            writeln!(
                out_f,
                "// @codegen-command : buck run fbcode//hphp/hack/src/hh_manual:hh_manual extract fbcode/hphp/hack/manual/hack/"
            )?;
        }

        writeln!(out_f, "{}", line)?;
    }

    Ok(())
}

/// Write all the extracted examples from page `page_name` to
/// `out_dir`.
fn write_extracted_examples(
    out_dir: &Path,
    page_rel_path: &Path,
    page_name: &str,
    code_blocks: &[CodeBlock],
) -> Result<()> {
    std::fs::create_dir_all(out_dir)?;

    let mut i = 0;
    for code_block in code_blocks {
        let out_name: String = if let Some(filename) = &code_block.filename {
            format!("{}-{}", page_name, filename)
        } else {
            i += 1;
            if code_block.error {
                format!("{}-{:02}.hack_error", page_name, i)
            } else {
                format!("{}-{:02}.hack", page_name, i)
            }
        };

        let out_path = out_dir.join(out_name);
        write_example(&out_path, &code_block.content, page_rel_path)
            .with_context(|| format!("Failed to write examples to {}", out_path.display()))?;
    }

    Ok(())
}

fn is_hidden(path: &Path) -> bool {
    let name = path.file_name().unwrap_or_default();
    name.to_string_lossy().starts_with('.')
}

/// Write all the extracted examples from `chapter_dir` to `test_dir`.
fn write_chapter_examples(chapter_dir: &Path, test_dir: &Path, hack_dir: &Path) -> Result<()> {
    let chapter_name = chapter_dir.file_name().unwrap().to_string_lossy();
    let out_dir = test_dir.join(&*chapter_name);

    let rel_chapter_dir = chapter_dir.strip_prefix(hack_dir).unwrap_or(chapter_dir);
    let rel_out_dir = out_dir.strip_prefix(hack_dir).unwrap_or(&out_dir);
    println!(
        "{:<45} -> {}",
        rel_chapter_dir.display(),
        rel_out_dir.display()
    );

    remove_existing_examples(&out_dir).with_context(|| {
        format!(
            "Failed to remove previously generated examples in {}",
            chapter_name
        )
    })?;

    for page_name in std::fs::read_dir(chapter_dir)? {
        let page_path = page_name?.path();

        if page_path.extension() == Some(&OsString::from("md")) && !is_hidden(&page_path) {
            let src_bytes = std::fs::read(&page_path)
                .with_context(|| format!("Could not read {}", page_path.display()))?;
            let src = String::from_utf8_lossy(&src_bytes);

            let code_blocks = extract_hack_blocks(&src)
                .with_context(|| format!("Page: {}", page_path.display()))?;
            let code_blocks = merge_by_filename(&code_blocks);

            let page_name = page_path.file_stem().unwrap().to_string_lossy();

            let page_rel_path = page_path.strip_prefix(hack_dir).unwrap_or(&page_path);
            write_extracted_examples(&out_dir, page_rel_path, &page_name, &code_blocks)?;
        }
    }

    Ok(())
}

/// Remove any *.php, *.hack or *.hack_error files from `path`, so we
/// don't have leftover extracted files that are no longer in the
/// markdown.
fn remove_existing_examples(path: &Path) -> Result<()> {
    if let Ok(dir_entries) = std::fs::read_dir(path) {
        for file_name in dir_entries {
            let file_path = file_name?.path();
            if file_path.extension() == Some(&OsString::from("php"))
                || (file_path.extension() == Some(&OsString::from("hack")))
                || (file_path.extension() == Some(&OsString::from("hack_error")))
            {
                std::fs::remove_file(file_path)?;
            }
        }
    }

    Ok(())
}

fn main() -> Result<()> {
    let cli = Cli::parse();

    let guide_dir = match cli.command {
        Commands::Extract { path } => path,
    };

    let abs_guide_dir = match guide_dir.canonicalize() {
        Ok(d) => d,
        Err(e) => match e.kind() {
            ErrorKind::NotFound => {
                return Err(anyhow::format_err!(
                    "Path does not exist: {}",
                    guide_dir.display()
                ));
            }
            _ => {
                return Err(anyhow::format_err!("{}", e.to_string()));
            }
        },
    };

    let hack_dir = abs_guide_dir.parent().unwrap().parent().unwrap();
    let test_dir = hack_dir.join("test").join("extracted_from_manual");

    for chapter_name in std::fs::read_dir(&abs_guide_dir)? {
        let chapter_path = chapter_name?.path();
        write_chapter_examples(&chapter_path, &test_dir, hack_dir)?;
    }

    Ok(())
}
