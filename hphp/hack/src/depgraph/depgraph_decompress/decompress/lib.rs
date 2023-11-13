use std::fs::File;
use std::fs::OpenOptions;
use std::io::BufWriter;
use std::io::Read;
use std::io::Write;
use std::ops::Deref;
use std::os::unix::fs::FileExt;
use std::path::Path;
use std::sync::atomic::AtomicUsize;
use std::sync::atomic::Ordering;

use depgraph_reader::compress::ByteApproximatedLen;
use depgraph_reader::compress::CompressedHeader;
use depgraph_reader::compress::RleBlock;
use depgraph_reader::compress::UncompressedHeader;
use depgraph_reader::Dep;
use rayon::prelude::*;

const IN_HEADER_SIZE: usize = std::mem::size_of::<CompressedHeader>();

/// Write the file header to the start of the output file.
///
/// See depgraph/depgraph_reader/compress.rs for the file format.
fn write_header(
    file: &File,
    num_deps: usize,
    adjacency_list_alignment_shift: u8,
) -> std::io::Result<()> {
    let final_header = UncompressedHeader {
        magic: UncompressedHeader::MAGIC,
        version: UncompressedHeader::LATEST_VERSION,
        num_deps: num_deps as u64,
        adjacency_list_alignment_shift,
        _alignment_padding: Default::default(),
    };

    file.write_all_at(bytemuck::bytes_of(&final_header), 0)
}

/// Write the `deps` and `deps_sort_order` sections.
///
/// See depgraph/depgraph_reader/compress.rs for the file format.
fn write_deps(file: &File, deps_bytes: &[u8]) -> std::io::Result<()> {
    const OUT_HEADER_SIZE: u64 = std::mem::size_of::<UncompressedHeader>() as u64;

    let (deps_write_status, sort_order_write_status) = rayon::join(
        || file.write_all_at(deps_bytes, OUT_HEADER_SIZE),
        || {
            // Create a sorted map of the Dep order, so we can binary search straight from the mmap file.
            let in_deps: &[Dep] = bytemuck::cast_slice(deps_bytes);
            let mut deps_sort_order: Vec<u32> = (0..in_deps.len() as u32).collect();
            deps_sort_order.par_sort_unstable_by_key(|&i| in_deps[i as usize]);

            file.write_all_at(
                bytemuck::cast_slice(&deps_sort_order),
                OUT_HEADER_SIZE + deps_bytes.len() as u64,
            )
        },
    );

    deps_write_status?;
    sort_order_write_status
}

/// Read in one variable-length integer from a stream.
fn read_vint64<R: Read>(mut r: R) -> std::io::Result<u64> {
    let mut buf = [0u8; 9];
    r.read_exact(&mut buf[..1])?;
    let len = vint64::decoded_len(buf[0]);
    r.read_exact(&mut buf[1..len])?;
    let mut rbuf = &buf[..];
    vint64::decode(&mut rbuf).map_err(|e| {
        std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            format!("Corrupt vint64: {}", e),
        )
    })
}

/// Convert a list of edges from the format used in `.zhhdg` files to the larger but random-access
/// format used in `.hhdg` files.
///
/// See `serialize_edge_list` for the writer.
fn decode_one_edge_list(mut b: &[u8]) -> Box<[u8]> {
    // Rough guess.
    let mut result = Vec::with_capacity(b.len() / 8);

    // Track the sum of sizes of all RLE blocks.
    let mut total_rle_decoded_len = 0u32;
    let mut num_rle_blocks = 0usize;

    let mut delta_base = 0u32;

    while !b.is_empty() {
        // Decode the (start, len) pair.
        let delta_and_has_repeat_count = vint64::decode(&mut b).unwrap();
        let has_repeat_count = (delta_and_has_repeat_count & 1) != 0;
        let delta = (delta_and_has_repeat_count >> 1) as u32;
        let mut len = if has_repeat_count {
            2 + vint64::decode(&mut b).unwrap() as u32
        } else {
            1
        };
        let mut start = delta_base + delta;
        delta_base = start + len + 1;

        total_rle_decoded_len += len;

        // Turn that into RleBlocks (usually just one, but each one's size is limited, so maybe more).
        while len != 0 {
            let encoded_len = ByteApproximatedLen::encode(len);
            let r = RleBlock { start, encoded_len };

            result.extend(bytemuck::bytes_of(&r));
            num_rle_blocks += 1;

            let num_consumed = encoded_len.decode();
            start += num_consumed;
            len -= num_consumed;
        }
    }

    // Prepend the RLE block count and the total length count.
    let old_len = result.len();
    result.extend(vint64::encode(num_rle_blocks as u64).as_ref());
    result.extend(vint64::encode(total_rle_decoded_len as u64).as_ref());
    result.rotate_left(old_len);

    result.into()
}

/// Decompress and parse the edge lists.
///
/// There are two levels of compression to undo here:
/// - Everything is compressed with zstd, so we need to decompress that first.
/// - Each edge array is delta-coded by the `.zhhdg` file writer. We need to
///   convert each one into the more verbose format that we use in `.hhdg` files.
fn decompress_edge_lists(in_edge_lists: &[u8]) -> std::io::Result<(Vec<Box<[u8]>>, usize)> {
    // First zstd-uncompress the raw edge list bytes. This is an inherently serial process.
    // Guess the output buffer is 1.33x as large as the input (we see 1.26x empirically).
    // If this number is too small, no big deal, zstd will grow the Vec.
    let mut unzstd_buf = Vec::with_capacity(in_edge_lists.len() * 4 / 3);
    let mut z = zstd::stream::read::Decoder::with_buffer(in_edge_lists)?;
    z.read_to_end(&mut unzstd_buf)?;
    drop(z);

    // Chop the edge list into individual edge arrays.
    let mut compressed_edge_lists: Vec<&[u8]> = vec![];
    let mut buf = &unzstd_buf[..];
    while !buf.is_empty() {
        let num_edge_bytes = vint64::decode(&mut buf).unwrap();
        let (edge_data, remaining_buf) = buf.split_at(num_edge_bytes.try_into().unwrap());
        buf = remaining_buf;
        compressed_edge_lists.push(edge_data);
    }

    // In parallel, convert each edge list from .zhhdg to .hhdg format.
    let total_bytes = AtomicUsize::new(0);
    let edge_lists: Vec<Box<[u8]>> = compressed_edge_lists
        .into_par_iter()
        .map(|v| {
            let d = decode_one_edge_list(v);
            total_bytes.fetch_add(d.len(), Ordering::Relaxed);
            d
        })
        .collect();

    Ok((edge_lists, total_bytes.into_inner()))
}

/// Guarantee that all edge list section offsets will be able to fit in 32 bits,
/// by choosing how many low bits of their sections offsets are required to be zero.
/// Those zero bits can be shifted away in our section offset table.
///
/// The graph would have to be exotically large for this to return anything
/// other than zero, but it's possible.
fn compute_alignment_shift(num_edge_lists: usize, total_bytes: usize) -> u8 {
    // Increase the guaranteed alignment of each serialized edge list so that
    // scaled offsets fit in 32 bits. We expect to typically have no shifting at
    // all, or maybe 1 bit someday (even-byte alignment), so this won't be very
    // wasteful.
    //
    // For example if we had 7GB of edge lists, and we wanted to be able to
    // reference each one with a 32-bit offset, we would align each edge list on
    // an even byte boundary and use an alignment_shift of 1, allowing 32 bits to
    // span 8GB of storage.  In other words, (unscaled_section_offset << 1) would
    // be the true section offset.
    //
    // The math here is slightly subtle -- increasing alignment also increases the
    // worst-case file size due to the new alignment padding, which in turn could
    // require increased alignment.  For example, if our section were 8 GiB minus 1 byte,
    // and we decided to increase alignment to make every offset even so we could
    // address 8GiB, that alignment padding may push us over 8GiB, so we need even
    // more alignment. We conservatively take that into account in our calculations by
    // assuming the worst-case padding is introduced for every edge list.
    let mut alignment_shift = 0;
    while (total_bytes + (num_edge_lists * ((1 << alignment_shift) - 1))) >> alignment_shift
        > std::u32::MAX as usize
    {
        alignment_shift += 1;
    }

    alignment_shift
}

/// Write the edge map, which maps a `HashIndex` to the section offset for its
/// list of edges.
fn write_edge_map(
    file: &File,
    num_deps: usize,
    in_edges_index: &[u8],
    edges_index_file_offset: u64,
    edge_lists: &[Box<[u8]>],
    alignment_shift: u8,
) -> std::io::Result<()> {
    // Start decompressing the edge indices.
    let mut z = zstd::stream::read::Decoder::with_buffer(in_edges_index)?;

    let mut buf = [0u8; 9];

    // Byte offset in the edge_list section for the next edge list.
    let mut out_offset = 0u64;

    let mut result: Vec<u32> = Vec::with_capacity(num_deps);
    let mut edge_list_index = 0;

    for i in 0..num_deps {
        // Read in the value that tells us whether this is a new edge list or a reference
        // to an earlier one. See `write_compressed_edge_map`.
        let n = read_vint64(&mut z)?;

        let val = if n != 0 {
            // This is a back reference, copy whatever the past entry was.
            result[i - n as usize]
        } else {
            // A new edge list; the corresponding edge_list section of the file will have
            // a new edge list appended to match.

            // Skip over any alignment padding to find the next serialized edge list.
            let pad = out_offset.wrapping_neg() & ((1 << alignment_shift) - 1);
            out_offset += pad;

            // Drop always-zero alignment bits, so the offset still fits in 32 bits.
            let shifted_offset: u32 = (out_offset >> alignment_shift).try_into().unwrap();

            // Advance the file offset to take into account this edge list's contents.
            out_offset += edge_lists[edge_list_index].len() as u64;
            edge_list_index += 1;

            shifted_offset
        };

        result.push(val);
    }

    // We should be exactly at EOF.
    if z.read(&mut buf[..1])? != 0 {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "Too many edge list index bytes",
        ));
    }

    file.write_all_at(bytemuck::cast_slice(&result), edges_index_file_offset)
}

/// Append the edge lists to the file at `path`.
fn write_edge_lists(
    path: &Path,
    edge_lists: &[Box<[u8]>],
    alignment_shift: u8,
) -> std::io::Result<()> {
    let file = OpenOptions::new().append(true).open(path)?;

    // We use a large buffer to see if it helps with some pathological
    // decompression speed behavior we're seeing.
    let mut buf = BufWriter::with_capacity(1024 * 1024, file);

    // Current byte offset into the `edge_lists` section of the file.
    let mut offset = 0u64;

    for edges in edge_lists.iter() {
        // Insert alignment padding. `write_edge_list_index` does equivalent
        // math so it computes the section offsets chosen here.
        while offset & ((1 << alignment_shift) - 1) != 0 {
            buf.write_all(&[0])?;
            offset += 1;
        }

        // It would be nice to use `write_all_vectored` and skip a layer
        // of copying, but it's still a nightly-only API.
        buf.write_all(edges)?;
        offset += edges.len() as u64;
    }

    // Flush `buf` and check for errors.
    buf.into_inner()?;

    Ok(())
}

/// Write both the `edge_lists_index` and the `edge_list` file sections.
///
/// Returns the alignment shift it computed, which needs to be added to the output
/// file header so readers know how to interpret the `edge_lists_index` section.
fn write_edges(
    file: &File,
    path: &Path,
    num_deps: usize,
    edges_index_file_offset: u64,
    in_edges_index: &[u8],
    in_edge_lists: &[u8],
) -> std::io::Result<u8> {
    // Decompress the edge list section, which we need to decompress the edge list index.
    let (edge_lists, total_edge_list_size) = decompress_edge_lists(in_edge_lists)?;

    // Figure out what kind of alignment padding we're going to need.
    let alignment_shift = compute_alignment_shift(edge_lists.len(), total_edge_list_size);

    let (edge_list_index_status, edge_list_status) = rayon::join(
        || {
            write_edge_map(
                file,
                num_deps,
                in_edges_index,
                edges_index_file_offset,
                &edge_lists,
                alignment_shift,
            )
        },
        || write_edge_lists(path, &edge_lists, alignment_shift),
    );

    edge_list_index_status?;
    edge_list_status?;

    Ok(alignment_shift)
}

/// Decompress the `.zhhdg` file at `in_path`, writing it to the `.hhdg` at `out_path`.
pub fn decompress(in_path: &Path, out_path: &Path) -> std::io::Result<()> {
    let in_file = File::open(in_path)?;

    // Safety: we rely on the memmap library to provide safety.
    let in_mmap = unsafe { memmap2::Mmap::map(&in_file) }?;
    drop(in_file);

    // We're going to read the entire file, so clue in the OS. We occasionally
    // see pathologically slow decompress times without this, so we're hoping this helps.
    let _ = in_mmap.advise(memmap2::Advice::WillNeed);

    let in_bytes = in_mmap.deref();
    if in_bytes.len() < IN_HEADER_SIZE {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "Missing file header",
        ));
    }

    let (header_bytes, in_rest) = in_bytes.split_at(std::mem::size_of::<CompressedHeader>());

    let header: &CompressedHeader = bytemuck::from_bytes(header_bytes);
    if header.magic != CompressedHeader::MAGIC {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            "Did not find expected file magic number",
        ));
    }

    let expected_version = CompressedHeader::LATEST_VERSION;
    if header.version != expected_version {
        return Err(std::io::Error::new(
            std::io::ErrorKind::InvalidData,
            format!(
                "Expected file version number {expected_version}, got {}",
                header.version
            ),
        ));
    }

    let num_deps = header.num_deps as usize;
    let deps_size = num_deps * std::mem::size_of::<Dep>();

    let (in_deps_bytes, in_rest) = in_rest.split_at(deps_size);
    let (in_edges_index, in_edge_lists) = in_rest.split_at(header.edge_map_size as usize);

    // The output file is divided into 5 sections.
    let out_header_size = std::mem::size_of::<UncompressedHeader>();
    let deps_order_size = num_deps * 4;
    let edge_map_size = num_deps * 4;

    // Create the output file. This size does not include the edge lists which we'll append;
    // we don't know their sizes yet.
    let edges_index_offset = out_header_size + deps_size + deps_order_size;
    let out_file_size = edges_index_offset + edge_map_size;
    let out_file = File::create(out_path)?;
    out_file.set_len(out_file_size as u64)?;

    // Write out the deps and edges sections in parallel.
    let (deps_status, edges_status) = rayon::join(
        || write_deps(&out_file, in_deps_bytes),
        || {
            write_edges(
                &out_file,
                out_path,
                num_deps,
                edges_index_offset as u64,
                in_edges_index,
                in_edge_lists,
            )
        },
    );

    deps_status?;
    let adjacency_list_alignment_shift = edges_status?;

    write_header(&out_file, num_deps, adjacency_list_alignment_shift)
}
