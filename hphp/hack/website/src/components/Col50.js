/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import React from 'react';

export default function Col50({children}) {
  return (
    <span
      style={{
        borderRadius: '0px',
        padding: '0rem',
        width: '50%',
        float: 'left',
        marginBottom: '0rem',
        paddingRight: '2rem',
      }}>
      {children}
    </span>
  );
}
