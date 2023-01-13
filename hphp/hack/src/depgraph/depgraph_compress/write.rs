// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs::File;
use std::fs::OpenOptions;
use std::io::BufWriter;
use std::io::Seek;
use std::io::SeekFrom;
use std::io::Write;
use std::ops::Range;
use std::path::Path;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;

use depgraph_reader::compress::CompressedHeader;
use depgraph_writer::HashIndex;
use depgraph_writer::HashListIndex;
use depgraph_writer::MemDepGraph;
use newtype::IdVec;
use rayon::prelude::*;
use zstd::stream::write::Encoder;

use crate::WriteConfig;

/// Convert an edge list into its final, RLE+delta-encoded form.
///
/// See `depgraph_reader/compress.rs` for a file format description.
fn serialize_edge_list(
    edge_list: &[HashIndex],
    most_rle_blocks: &AtomicUsize,
    total_rle_blocks: &AtomicUsize,
    config: &WriteConfig,
) -> Box<[u8]> {
    // Rough capacity guess, probably wrong.
    let mut rle_blocks: Vec<u8> = Vec::with_capacity(1 + edge_list.len() * 10);

    let mut num_rle_blocks = 0usize;
    let mut prev = 0u32;

    let mut flush_range = |b: Range<u32>| {
        match config {
            WriteConfig::Simple => {
                rle_blocks.extend(b.start.to_ne_bytes());
                rle_blocks.extend(b.end.to_ne_bytes());
            }
            WriteConfig::Zstd { .. } => {
                // Use our own delta-coding.
                let delta = b.start - prev;
                let has_repeat_count = b.len() > 1;

                let delta_and_has_repeat_count: u64 =
                    (delta as u64) << 1 | (has_repeat_count as u64);
                rle_blocks.extend(vint64::encode(delta_and_has_repeat_count).as_ref());
                if has_repeat_count {
                    rle_blocks.extend(vint64::encode((b.len() - 2) as u64).as_ref());
                }

                // There must be a gap, or it would have been a larger block, so
                // we can move `prev` one past `b.end`.
                prev = b.end + 1;
            }
        }

        num_rle_blocks += 1;
    };

    // Convert the edge_list into a sequence of Ranges, which we RLE-encode.
    let mut edge_list_iter = edge_list.iter().copied();
    if let Some(HashIndex(first_edge)) = edge_list_iter.next() {
        // Establish a starting point.
        let mut cur = Range {
            start: first_edge,
            end: first_edge + 1,
        };

        // Process the remaining edges, hopefully combining them into RLE blocks.
        for HashIndex(edge) in edge_list_iter {
            if cur.end == edge {
                // They are contiguous, so expand the current block.
                cur.end += 1;
            } else {
                // Noncontiguous; flush the previous block and start a new one.

                // These had better be sorted.
                assert!(edge > cur.end);

                flush_range(cur);

                cur = Range {
                    start: edge,
                    end: edge + 1,
                }
            }
        }

        // Flush the final range.
        flush_range(cur);
    }

    // Update stats.
    total_rle_blocks.fetch_add(num_rle_blocks, Ordering::Relaxed);
    most_rle_blocks.fetch_max(num_rle_blocks, Ordering::Relaxed);

    // Prepend each edge list with the number of bytes, rather than the number of blocks.  This
    // is a bit larger, but makes decompression easier since it can immediately tell how many
    // bytes each record needs.
    let encoded_len = vint64::encode(rle_blocks.len() as u64);
    let encoded_num_rle_blocks = (num_rle_blocks as u32).to_ne_bytes();

    let encoded_len = match config {
        WriteConfig::Simple => &encoded_num_rle_blocks,
        WriteConfig::Zstd { .. } => encoded_len.as_ref(),
    };

    // Create a final Vec of exactly the right size, so no additional copying
    // happens when we convert it to a Boxed slice.
    let mut result = Vec::with_capacity(encoded_len.len() + rle_blocks.len());
    result.extend(encoded_len);
    result.extend(rle_blocks);

    result.into()
}

fn write_hashes(file: &mut File, m: &MemDepGraph) -> std::io::Result<u64> {
    let mut out = BufWriter::new(file);

    // Leave padding for the CompressedHeader we'll fill in later.
    for _ in 0..std::mem::size_of::<CompressedHeader>() {
        out.write_all(&[0])?;
    }

    out.write_all(bytemuck::cast_slice(&m.hashes))?;

    // Let `into_inner()` flush here, so we notice disk full etc.
    let file = out.into_inner()?;

    // Return the number of bytes written (including the header padding).
    file.stream_position()
}

/// This writes out the edge list index, which has one entry for every hash.
/// Entries are variable-length encoded; they aren't intended to be random-accessed
/// in .zhhdg files,  only decompressed.
///
/// There are two possibilities for each entry:
///
/// - 0: This is the first time a particular edge list has appeared.
///   A new entry has been appended to the `edge_lists` section to match.
/// - Other (N >= 1): This is the same as some previous edge list. Look backwards N
///   slots to find the previous edge list with the same contents.
///
/// This is related to move-to-front coding, the idea is to use small numbers for consecutive
/// or nearby repeats, which are common due to how `apply_node_renumbering` assigns values.
fn write_compressed_edge_map<W: Write>(
    out: &mut W,
    m: &MemDepGraph,
) -> std::io::Result<Vec<HashListIndex>> {
    let mut ser_order = Vec::with_capacity(m.edge_lists.len());

    const NOT_EMITTED: u32 = u32::MAX;
    let mut most_recent: IdVec<HashListIndex, u32> =
        IdVec::new_from_vec(vec![NOT_EMITTED; m.edge_lists.len()]);
    for (i, &edge_list_index) in m.edge_list_indices.iter().enumerate() {
        let slot = &mut most_recent[edge_list_index];
        let latest = std::mem::replace(slot, i as u32);
        let delta = if latest == NOT_EMITTED {
            ser_order.push(edge_list_index);

            // A delta of 0 means we are introducing a new edge list here.
            0
        } else {
            // This is a back-reference to the last time we emitted this `HashListIndex`.
            i as u32 - latest
        };

        out.write_all(vint64::encode(delta as u64).as_ref())?;
    }

    Ok(ser_order)
}

/// For experimentation we don't compress here, just write out simple u32 values.
fn write_simple_edge_list_index(file: &mut File, m: &MemDepGraph) -> std::io::Result<()> {
    file.write_all(bytemuck::cast_slice(&m.edge_list_indices.vec[..]))
}

/// Append both the edge lists index and the edge lists themselves to `path`.
///
/// Returns the byte size of the edges_index section.
fn write_edges(
    path: &Path,
    edges_index_start: u64,
    m: &MemDepGraph,
    config: &WriteConfig,
) -> std::io::Result<u64> {
    // Open the file again, so we can write to a different section in parallel.
    let mut file = OpenOptions::new().write(true).open(path)?;
    file.seek(SeekFrom::Start(edges_index_start))?;

    let (edge_index_status, ser_edge_lists) = rayon::join(
        || -> std::io::Result<(Option<Vec<HashListIndex>>, File)> {
            // Write out the edge list index section.
            match config {
                WriteConfig::Simple => {
                    write_simple_edge_list_index(&mut file, m)?;
                    Ok((None, file))
                }
                WriteConfig::Zstd { compression_level } => {
                    let mut z = Encoder::new(file, *compression_level)?;
                    z.multithread((std::cmp::max(rayon::current_num_threads(), 2) - 1) as u32)?;
                    let ser_order = write_compressed_edge_map(&mut z, m)?;
                    let file = z.finish()?;
                    Ok((Some(ser_order), file))
                }
            }
        },
        || {
            // Compute the serialized edge_lists. We'll write them out below, after
            // the file writes above have finished.

            // Gather some statistics for logging.
            let most_rle_blocks = AtomicUsize::new(0);
            let total_rle_blocks = AtomicUsize::new(0);
            let total_size = AtomicUsize::new(0);

            let ser_edge_lists_vec: Vec<Box<[u8]>> = m
                .edge_lists
                .par_iter()
                .with_min_len(1)
                .with_max_len(1)
                .map(|edge_list| {
                    let edges =
                        serialize_edge_list(edge_list, &most_rle_blocks, &total_rle_blocks, config);
                    total_size.fetch_add(edges.len(), Ordering::Relaxed);
                    edges
                })
                .collect();
            let ser_edge_lists: IdVec<HashListIndex, Box<[u8]>> =
                IdVec::new_from_vec(ser_edge_lists_vec);

            log::info!(
                "Uncompressed edge list size {}, RLE blocks {}, most RLE blocks in an edge list is {}",
                total_size.into_inner(),
                total_rle_blocks.into_inner(),
                most_rle_blocks.into_inner()
            );

            ser_edge_lists
        },
    );

    let (ser_order, mut file) = edge_index_status?;

    // Remember where the edge index ends, and therefore the edge list starts.
    let edges_index_end = file.stream_position()?;
    let edge_map_size = edges_index_end - edges_index_start;

    if log::log_enabled!(log::Level::Info) {
        log::info!("Compressed edge list indices take {} bytes", edge_map_size);

        if ser_edge_lists.len() > 10 {
            // Find the biggest 10. No need to sort the entire vec.
            let mut sizes: Vec<_> = ser_edge_lists.iter().map(|s| s.len()).collect();
            let (_, _, biggest_10) = sizes.select_nth_unstable(ser_edge_lists.len() - 11);
            biggest_10.sort_by_key(|&i| std::cmp::Reverse(i));

            log::info!(
                "Top serialized edge_list byte sizes are {}",
                biggest_10
                    .iter()
                    .map(|&x| format!("{}", x))
                    .collect::<Vec<_>>()
                    .join(", ")
            );
        }
    }

    // Write a separate compressed section for the edge lists.
    let mut file = match config {
        WriteConfig::Simple => {
            let mut buf = BufWriter::new(file);
            for ser_edge_list in ser_edge_lists.iter() {
                buf.write_all(ser_edge_list)?;
            }
            buf.into_inner()?
        }
        WriteConfig::Zstd { compression_level } => {
            let mut z = Encoder::new(file, *compression_level)?;
            z.multithread((std::cmp::max(rayon::current_num_threads(), 2) - 1) as u32)?;

            for ser_index in ser_order.unwrap() {
                z.write_all(&ser_edge_lists[ser_index])?;
            }

            z.finish()?
        }
    };

    let edge_lists_end = file.stream_position()?;
    drop(file);

    log::info!(
        "Compressed edge lists take {} bytes",
        edge_lists_end - edges_index_end
    );

    Ok(edge_map_size)
}

fn write_header(
    file: &mut File,
    num_deps: u64,
    edge_map_size: u64,
    config: &WriteConfig,
) -> std::io::Result<()> {
    let magic = match config {
        WriteConfig::Simple => {
            // Use a nonstandard magic number so we know the file isn't readable.
            *b"HHZS"
        }
        WriteConfig::Zstd { .. } => CompressedHeader::MAGIC,
    };

    let header = CompressedHeader {
        magic,
        version: CompressedHeader::LATEST_VERSION,
        num_deps,
        edge_map_size,
    };

    // Write the header at the very beginning of the file.
    file.rewind()?;
    file.write_all(bytemuck::bytes_of(&header))
}

/// Write a `MemDepGraph` to the given path as a `.zhhdg` file.
///
/// See `depgraph_reader/compress.rs` for a file format description.
pub(crate) fn write_to_disk(
    path: &Path,
    m: MemDepGraph,
    config: &WriteConfig,
) -> std::io::Result<()> {
    log::info!("Writing file contents");

    let header_size = std::mem::size_of::<CompressedHeader>() as u64;
    let num_deps = m.hashes.len() as u64;
    let edge_data_start = header_size + num_deps * 8;

    // Create the file, and grow it now so we can write deps and append edge_list data in parallel.
    let mut file = std::fs::File::create(path)?;
    file.set_len(edge_data_start)?;

    // Write the dep table (which is most of the file) and the edge data (which needs compression)
    // in parallel, to different sections of the file.
    let (num_dep_table_bytes_written, edge_lists_write_result) = rayon::join(
        || write_hashes(&mut file, &m),
        || write_edges(path, edge_data_start, &m, config),
    );

    // Sanity check that the dep section contains exactly the expected number of bytes.
    let num_dep_table_bytes_written = num_dep_table_bytes_written?;
    assert_eq!(num_dep_table_bytes_written, edge_data_start);

    let edge_map_size = edge_lists_write_result?;

    // Now that we know various file parameters, go back and write the header at the file start.
    write_header(&mut file, num_deps, edge_map_size, config)?;

    let file_size = file.metadata().unwrap().len();
    drop(file);

    log::info!(
        "Writing depgraph file done. File size is {} bytes",
        file_size
    );

    Ok(())
}
