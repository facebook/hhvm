module Utils (floatOfBits, floatBits) where

import Data.Int
import Data.Word
import GHC.Exts
import GHC.Prim
import GHC.Word

floatBits :: Double -> Word64
floatBits (D# d#) = W64# (unsafeCoerce# d#)

floatOfBits :: Word64 -> Double
floatOfBits (W64# b#) = D# (unsafeCoerce# b#)
