/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

import React from 'react';
import Link from '@docusaurus/Link';
import styles from './Toc.module.css';
import clsx from 'clsx';

export function TocColumn({data, col}) {
  const sections = data.filter(s => s.column == col + 1);
  return (
    <div className={clsx('col col--4', styles.feature)}>
      {sections.map(s =>
        <div>
          <h4 className={styles.tocSection}>
            {s.section}
          </h4>
          {s.entries.map(e =>
            <p className={styles.tocEntry}>
              <Link to={e.to}>
                {e.label}
              </Link>
            </p>
          )}
        </div>
      )}
    </div>
  );
  return <p>{col}</p>;
}

export function Main({title, blocks, cols}) {
  return (
    <div>
      <h2 className={styles.tocTable}>
        {title}
      </h2>
      {blocks.map(b =>
        <div className={styles.tocTable}>
          <div className="row">
            {[0, 1, 2].map((e, i) =>
              <TocColumn data={b} col={i} />
            )}
          </div>
        </div>
      )}
    </div>
  );
}
