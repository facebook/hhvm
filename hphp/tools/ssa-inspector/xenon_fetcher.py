# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
Fetch Xenon gCPU data from the xenon Scuba dataset.

Provides exclusive (leaf) CPU % for PHP functions, matching the fn_name
column format used by tcprint (ClassName::methodName).
"""

from __future__ import annotations

from analytics.bamboo import Bamboo as bb


def _escape_scuba_sql(s: str) -> str:
    """Escape a string for use in a Scuba SQL string literal.

    Backslashes in PHP namespaces (e.g. FlibSL\\C\\contains_key) need
    to be doubled in Scuba SQL.
    """
    return s.replace("\\", "\\\\").replace("'", "\\'")


def fetch_gcpu_for_functions(
    function_names: list[str],
    lookback_hours: int = 4,
) -> dict[str, float]:
    """Return {func_name: gcpu_pct} for each function.

    Queries xenon for exclusive CPU % (leaf cost) using the fn_name column.
    Returns percentages as floats (e.g. 0.12 means 0.12% of total CPU).
    """
    if not function_names:
        return {}

    query = (
        f"SELECT `fn_name`, COUNT(1) AS `sample_count` "
        f"FROM `xenon` "
        f"WHERE `time` >= NOW() - {lookback_hours} * 3600 "
        f"AND `is_io` = '0' "
        f"GROUP BY `fn_name` "
        f"ORDER BY `sample_count` DESC "
        f"LIMIT 500"
    )

    df = bb.query_scuba_nullable(sql=query)

    if df.empty:
        return {}

    # Build a map of fn_name -> sample_count from the results
    total_samples = int(df["sample_count"].sum())
    if total_samples == 0:
        return {}

    name_to_count: dict[str, int] = dict(
        zip(df["fn_name"].astype(str), df["sample_count"].astype(int))
    )

    # Compute gCPU % for requested functions
    result: dict[str, float] = {}
    for func in function_names:
        count = name_to_count.get(func, 0)
        if count > 0:
            result[func] = round(count / total_samples * 100, 4)

    return result
