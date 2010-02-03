{-# INCLUDE "hphp_ffi.h" #-}
{-# LANGUAGE ForeignFunctionInterface #-}
module HphpFFI (Variant, VariantOrd, VariantAble(..), VariantOrdAble(..),
               vToMap, vToInt, vToDouble, vToString, vToBool, vIsNull,
               withExportedVariant, HphpVariantPtr, buildVariant, PtrPtr,
                hphpIncludeFile, hphpFfiInit, hphpFreeGlobals,
                withVParamList, fullyLocalize) where
import Utils
import Data.Int
import Foreign.C
import Foreign.ForeignPtr
import Foreign.Ptr
import Foreign.Storable (peek)
import Foreign.Marshal.Alloc (alloca)
import Data.Maybe (fromJust)
import qualified Data.ByteString.Char8 as B
import qualified Data.Map as M


-- Raw pointers to hphp data
data ArrayData = ArrayData
data StringData = StringData
data ObjectData = ObjectData
data HphpVariant = HphpVariant
type HphpVariantPtr = Ptr HphpVariant

type ArrayDataPtr = Ptr ArrayData
type StringDataPtr = Ptr StringData
type ObjectDataPtr = Ptr ObjectData

type PtrPtr a = Ptr (Ptr a)

-- Foreign pointers for hphp data pointers
type ArrayDataHnd = ForeignPtr ArrayData
type StringDataHnd = ForeignPtr StringData
type ObjectDataHnd = ForeignPtr ObjectData


-- Using foreign ptrs for hooking into hphp's ref counting
foreign import ccall "hphp_ffi.h &ArrayData_decRef" arrayData_decRef
    :: FunPtr (ArrayDataPtr -> IO ())
foreign import ccall "hphp_ffi.h &StringData_decRef" stringData_decRef
    :: FunPtr (StringDataPtr -> IO ())
foreign import ccall "hphp_ffi.h &ObjectData_decRef" objectData_decRef
    :: FunPtr(ObjectDataPtr -> IO ())

introduceArrayData :: Ptr ArrayData -> IO ArrayDataHnd
introduceArrayData ad = newForeignPtr arrayData_decRef ad
introduceStringData :: Ptr StringData -> IO StringDataHnd
introduceStringData ad = newForeignPtr stringData_decRef ad
introduceObjectData :: Ptr ObjectData -> IO ObjectDataHnd
introduceObjectData ad = newForeignPtr objectData_decRef ad


-- Various utility ccalls
foreign import ccall "hphp_ffi.h buildVariant" buildHphpVariant
    :: CInt -> Ptr a -> CInt -> IO HphpVariantPtr
foreign import ccall "hphp_ffi.h freeVariant" freeHphpVariant
    :: HphpVariantPtr -> IO ()

foreign import ccall "hphp_ffi.h addMapItem" addHphpMapItem
    ::  HphpVariantPtr -> HphpVariantPtr -> HphpVariantPtr -> IO ()

foreign import ccall "hphp_ffi.h iter_begin" adIterBegin
    ::  ArrayDataPtr -> IO CLLong
foreign import ccall "hphp_ffi.h iter_advance" adIterAdvance
    ::  ArrayDataPtr -> CLLong -> IO CLLong
foreign import ccall "hphp_ffi.h iter_end" adIterEnd
    ::  ArrayDataPtr -> IO CLLong
foreign import ccall "hphp_ffi.h iter_invalid" _adIterInvalid
    ::  CLLong -> IO CInt
adIterInvalid :: CLLong -> IO Bool
adIterInvalid p = do
  r <- _adIterInvalid p
  return $ r == 1
foreign import ccall "hphp_ffi.h iter_getKey" adIterGetKey
    ::  ArrayDataPtr -> CLLong -> PtrPtr a -> IO CInt
foreign import ccall "hphp_ffi.h iter_getValue" adIterGetValue
    ::  ArrayDataPtr -> CLLong -> PtrPtr a -> IO CInt

foreign import ccall "hphp_ffi.h string_data" sdGetData
    ::  StringDataPtr -> Ptr CString -> IO CInt

foreign import ccall "hphp_ffi.h include_file" _hphpIncludeFile
    ::  CString -> IO ()

foreign import ccall "hphp_ffi.h ffi_init" hphpFfiInit
    ::  IO ()
foreign import ccall "hphp_ffi.h ffi_free_globals" hphpFreeGlobals
    ::  IO ()

hphpIncludeFile f = do
  s <- newCString f
  _hphpIncludeFile s

-- Variant type
data VariantOrd = VOInt Int64
                | VOString B.ByteString
                  deriving (Eq, Ord, Show)

data Variant = VNull
             | VInt Int64
             | VDbl Double
             | VBool Bool
             | VBString B.ByteString
             | VMap (M.Map VariantOrd Variant)
             | VLitStr CString
             | VString StringDataHnd
             | VArray ArrayDataHnd
             | VObject ObjectDataHnd
               deriving Show

ptrToInt :: Ptr a -> Int64
ptrToInt p = fromIntegral . ptrToIntPtr $ p

ptrToDbl :: Ptr a -> Double
ptrToDbl p = floatOfBits (fromIntegral . ptrToIntPtr $ p)

buildVariant :: Int -> Ptr a -> IO Variant
buildVariant typ ptr =
  case typ of
    0 -> return VNull
    1 -> return $ VBool ((ptrToInt ptr) /= 0)
    2 -> return $ VInt (ptrToInt ptr)
    3 -> return $ VDbl (ptrToDbl ptr)
    4 -> return $ VLitStr (castPtr ptr)
    5 -> (return . VString) =<<
           (introduceStringData (castPtr ptr))
    6 -> (return . VArray) =<<
           (introduceArrayData (castPtr ptr))
    7 -> (return . VObject) =<<
           (introduceObjectData (castPtr ptr))

sdToBs :: StringDataHnd -> IO B.ByteString
sdToBs sd = alloca $ \pd -> do
  len <- withForeignPtr sd (\psd -> sdGetData psd pd)
  cs <- peek pd
  B.packCStringLen (cs, fromIntegral len)


foldAd :: ArrayDataHnd -> CLLong -> (VariantOrd -> Variant -> b -> b)
       -> b -> IO b
foldAd ad pos f init = do
  inv <- adIterInvalid pos
  if inv then return init else do
      k <- alloca $ \pk -> do
                         tk <- withForeignPtr ad (\adp ->
                                                      adIterGetKey adp pos pk)
                         pkv <- peek pk
                         vk <- buildVariant (fromIntegral tk) pkv
                         fromJust $ variantToVariantOrd vk
      v <- alloca $ \pv -> do
                         tv <- withForeignPtr ad (\adp ->
                                                      adIterGetValue adp pos pv)
                         pvv <- peek pv
                         buildVariant (fromIntegral tv) pvv
      npos <- withForeignPtr ad (\adp -> adIterAdvance adp pos)
      foldAd ad npos f (f k v init)

adToDm :: ArrayDataHnd -> IO (M.Map VariantOrd Variant)
adToDm ad = do
  beg <- withForeignPtr ad (\adp -> adIterBegin adp)
  foldAd ad beg M.insert M.empty

variantToVariantOrd :: Variant -> Maybe (IO VariantOrd)
variantToVariantOrd (VBString b) = Just . return $ VOString b
variantToVariantOrd (VInt i) = Just . return $ VOInt i
variantToVariantOrd (VString s) = Just $ do
  bs <- sdToBs s
  return  (VOString bs)
variantToVariantOrd _ = Nothing

class VariantAble a where
    toVariant :: a -> Variant
class VariantOrdAble a where
    toVariantOrd :: a -> VariantOrd

instance VariantAble () where
    toVariant () = VNull
instance VariantAble Bool where
    toVariant b = VBool b
instance VariantAble Int where
    toVariant i = VInt (fromIntegral i)
instance VariantAble Int64 where
    toVariant i = VInt i
instance VariantAble String where
    toVariant s = VBString (B.pack s)
instance VariantAble Double where
    toVariant d = VDbl d
instance VariantAble B.ByteString where
    toVariant b = VBString b
instance (VariantOrdAble k, VariantAble v) => VariantAble (M.Map k v) where
    toVariant m = VMap $ M.map toVariant (M.mapKeys toVariantOrd m)
instance VariantAble VariantOrd where
    toVariant (VOInt i) = VInt i
    toVariant (VOString bs) = VBString bs
instance VariantAble Variant where
    toVariant v = v

instance VariantOrdAble Int where
    toVariantOrd i = VOInt (fromIntegral i)
instance VariantOrdAble Int64 where
    toVariantOrd i = VOInt i
instance VariantOrdAble String where
    toVariantOrd s = VOString (B.pack s)
instance VariantOrdAble B.ByteString where
    toVariantOrd b = VOString b


intToVp :: Integral a => a -> Ptr b
intToVp i = intPtrToPtr (fromIntegral i)

dblToVp :: Double -> Ptr a
dblToVp d = intToVp (floatBits d)

exportVariant :: Variant -> IO HphpVariantPtr
exportVariant VNull = buildHphpVariant 0 nullPtr 0
exportVariant (VBool v) = buildHphpVariant 1 (intToVp (if v then 1 else 0)) 0
exportVariant (VInt i) = buildHphpVariant 2 (intToVp i) 0
exportVariant (VDbl d) = buildHphpVariant 3 (dblToVp d) 0
exportVariant (VLitStr cs) = buildHphpVariant 4 cs 0
exportVariant (VString s) = withForeignPtr s (\p -> buildHphpVariant 5 p 0)
exportVariant (VBString bs) = B.useAsCStringLen bs
                              (\(cs, l) ->
                                   buildHphpVariant 6 cs (fromIntegral l))
exportVariant (VArray a) = withForeignPtr a (\p -> buildHphpVariant 7 p 0)
exportVariant (VObject o) = withForeignPtr o (\p -> buildHphpVariant 8 p 0)
exportVariant (VMap m) = do
  p <- buildHphpVariant 0 nullPtr 0
  M.foldWithKey (adder p) (return ()) m
  return p
    where
      adder p k v _ = do
        kv <- exportVariant (toVariant k)
        vv <- exportVariant v
        addHphpMapItem p kv vv
        freeHphpVariant kv
        freeHphpVariant vv

withExportedVariant :: VariantAble v => v -> (HphpVariantPtr -> IO a) -> IO a
withExportedVariant v f = do
  vp <- exportVariant (toVariant v)
  r <- f vp
  freeHphpVariant vp
  return r

withVParamList :: [Variant] -> (HphpVariantPtr -> IO a) -> IO a
withVParamList params = withExportedVariant
                        (VMap . M.mapKeys toVariantOrd . M.fromList $
                              zip [(0 :: Int64)..] params)

-- Variant Conversions

vToMap :: Variant -> Maybe (IO (M.Map VariantOrd Variant))
vToMap (VMap m) = Just (return m)
vToMap (VArray ad) = Just $ adToDm ad
vToMap _ = Nothing

vToInt :: Variant -> Maybe Int64
vToInt (VInt i) = Just i
vToInt _ = Nothing

vToDouble :: Variant -> Maybe Double
vToDouble (VDbl d) = Just d
vToDouble _ = Nothing

vToString :: Variant -> Maybe (IO B.ByteString)
vToString (VBString bs) = Just . return $ bs
vToString (VLitStr cs) = Just (B.packCString cs)
vToString (VString sd) = Just $ sdToBs sd
vToString _ = Nothing

vToBool :: Variant -> Maybe Bool
vToBool (VBool b) = Just b
vToBool _ = Nothing

vIsNull :: Variant -> Bool
vIsNull VNull = True
vIsNull _ = False

fullyLocalize :: Variant -> IO Variant
fullyLocalize (VString sd) = do
  bs <- sdToBs sd
  return $ VBString bs
fullyLocalize (VArray ad) = do
  dm <- adToDm ad
  dm2 <- M.foldWithKey (\k v mm -> do
                          m <- mm
                          vl <- fullyLocalize v
                          return $ M.insert k vl m)
         (return M.empty) dm
  return $ VMap dm2
fullyLocalize v = return v

