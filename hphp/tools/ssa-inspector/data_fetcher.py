# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
Fetch tcprint SSA IR data from Hive via Presto, or from local trace files.

Hive table: infrastructure.hhvm_tcprint_data
  Columns: trans_id (BIGINT), data (STRING/JSON blob)
  Partitions: ds (date string), run_uuid (string)
"""

from __future__ import annotations

import json
from typing import Optional

from datainfra.presto.client_lib import getPrismPrestoClient


HIVE_TABLE = "infrastructure.hhvm_tcprint_data"
PRESTO_NAMESPACE = "infrastructure"
PRESTO_SOURCE = "ssa-inspector"


def _run_presto_query(query: str) -> list[dict]:
    """Execute a Presto query and return rows as list of dicts."""
    with getPrismPrestoClient(
        namespace=PRESTO_NAMESPACE,
        query=query,
        source=PRESTO_SOURCE,
        use_authentication=True,
    ) as client:
        return list(client.execute())


def list_runs(days: int = 7) -> list[dict]:
    """List available tcprint runs from the last N days."""
    query = f"""
        SELECT DISTINCT ds, run_uuid
        FROM {HIVE_TABLE}
        WHERE ds >= date_format(date_add('day', -{days}, current_date), '%Y-%m-%d')
        ORDER BY ds DESC
        LIMIT 20
    """
    return _run_presto_query(query)


def _build_partition_filter(
    ds: Optional[str] = None,
    run_uuid: Optional[str] = None,
    tenant: Optional[str] = None,
    partition: Optional[int] = None,
) -> str:
    """Build WHERE clause fragments for partition filtering."""
    clauses = []
    if ds is not None:
        clauses.append(f"ds = '{ds}'")
    if run_uuid is not None:
        clauses.append(f"run_uuid = '{run_uuid}'")
    elif tenant is not None:
        pattern = tenant
        if partition is not None:
            pattern = f"{tenant}.cln.{partition}"
        clauses.append(f"run_uuid LIKE '%{pattern}%'")
    if not clauses:
        # Default: latest date
        clauses.append(
            f"ds = (SELECT MAX(ds) FROM {HIVE_TABLE} "
            f"WHERE ds >= date_format(date_add('day', -7, current_date), '%Y-%m-%d'))"
        )
    return " AND ".join(clauses)


def fetch_top_translations(
    top_n: int = 10,
    ds: Optional[str] = None,
    run_uuid: Optional[str] = None,
    tenant: Optional[str] = None,
    partition: Optional[int] = None,
) -> list[dict]:
    """Fetch top N translations by profCount (using first block as proxy).

    Returns list of dicts with keys: trans_id, func_name, prof_count, data.
    Two-step query: first rank by profCount, then fetch full data for top N.
    """
    part_filter = _build_partition_filter(ds, run_uuid, tenant, partition)

    # Step 1: rank translations by first block's profCount
    ranking_query = f"""
        SELECT
            trans_id,
            json_extract_scalar(data, '$.translation.funcName') as func_name,
            json_extract_scalar(data, '$.translation.kind') as kind,
            COALESCE(
                CAST(json_extract_scalar(data, '$.blocks[0].profCount') AS BIGINT),
                0
            ) as prof_count
        FROM {HIVE_TABLE}
        WHERE {part_filter}
        ORDER BY prof_count DESC
        LIMIT {top_n}
    """
    ranking = _run_presto_query(ranking_query)
    if not ranking:
        return []

    # Step 2: fetch full data for top trans_ids
    ids = ", ".join(str(r["trans_id"]) for r in ranking)
    data_query = f"""
        SELECT trans_id, data
        FROM {HIVE_TABLE}
        WHERE {part_filter}
          AND trans_id IN ({ids})
    """
    data_rows = _run_presto_query(data_query)
    data_by_id = {r["trans_id"]: r["data"] for r in data_rows}

    # Merge ranking info with full data
    results = []
    for r in ranking:
        tid = r["trans_id"]
        results.append(
            {
                "trans_id": tid,
                "func_name": r["func_name"],
                "kind": r.get("kind", ""),
                "prof_count": r["prof_count"],
                "data": data_by_id.get(tid, "{}"),
            }
        )
    return results


def fetch_translations_by_function(
    function_name: str,
    ds: Optional[str] = None,
    run_uuid: Optional[str] = None,
    tenant: Optional[str] = None,
    partition: Optional[int] = None,
    kind: Optional[str] = None,
) -> list[dict]:
    """Fetch translations matching a function name pattern.

    Returns list of dicts with keys: trans_id, data.
    """
    part_filter = _build_partition_filter(ds, run_uuid, tenant, partition)
    kind_filter = ""
    if kind is not None:
        kind_filter = f" AND json_extract_scalar(data, '$.translation.kind') = '{kind}'"

    query = f"""
        SELECT trans_id, data
        FROM {HIVE_TABLE}
        WHERE {part_filter}
          AND json_extract_scalar(data, '$.translation.funcName')
              LIKE '%{function_name}%'
          {kind_filter}
    """
    return _run_presto_query(query)


def fetch_translation_by_id(
    trans_id: int,
    ds: Optional[str] = None,
    run_uuid: Optional[str] = None,
    tenant: Optional[str] = None,
    partition: Optional[int] = None,
) -> Optional[dict]:
    """Fetch a single translation by its ID.

    Returns dict with keys: trans_id, data; or None if not found.
    """
    part_filter = _build_partition_filter(ds, run_uuid, tenant, partition)
    query = f"""
        SELECT trans_id, data
        FROM {HIVE_TABLE}
        WHERE {part_filter}
          AND trans_id = {trans_id}
    """
    rows = _run_presto_query(query)
    return rows[0] if rows else None


def load_from_local_file(
    path: str,
    function_filter: Optional[str] = None,
    trans_id_filter: Optional[int] = None,
) -> list[dict]:
    """Load translations from a local trace file.

    Supports two formats:
    - Hive data format: {trans_id}\\x01{json_blob} per line
    - Raw JSON: one JSON object per line (auto-assigned sequential trans_id)

    Lines starting with 'json:' have the prefix stripped before parsing.
    """
    results: list[dict] = []
    auto_id = 0

    with open(path) as f:
        for line in f:
            line = line.rstrip("\n")
            if not line:
                continue

            # Strip 'json:' prefix if present
            if line.startswith("json:"):
                line = line[5:]

            # Try Hive data format: trans_id\x01json_blob
            if "\x01" in line:
                parts = line.split("\x01", 1)
                try:
                    tid = int(parts[0])
                    json_str = parts[1]
                except (ValueError, IndexError):
                    continue
            else:
                # Raw JSON line
                json_str = line
                tid = auto_id
                auto_id += 1

            try:
                data = json.loads(json_str)
            except json.JSONDecodeError:
                continue

            # Apply filters
            if trans_id_filter is not None and tid != trans_id_filter:
                continue

            if function_filter is not None:
                func_name = data.get("translation", {}).get("funcName", "")
                if function_filter not in func_name:
                    continue

            results.append({"trans_id": tid, "data": json_str})

    return results
