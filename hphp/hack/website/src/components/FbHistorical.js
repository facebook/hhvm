/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import React from 'react';
import {OssOnly, FbInternalOnly} from 'docusaurus-plugin-internaldocs-fb/internal';
import MDXContent from '@theme/MDXContent';
import Admonition from '@theme/Admonition';

export default function FbHistorical({children}) {
  return (
    <FbInternalOnly>
    <Admonition type="info" title="Historical Note (applies in FB WWW repository)">
      <MDXContent>
        {children}
      </MDXContent>
    </Admonition>
    </FbInternalOnly>
  );
}
