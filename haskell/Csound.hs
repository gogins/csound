{-# LANGUAGE DataKinds, QuasiQuotes, EmptyDataDecls, ExtendedDefaultRules, ForeignFunctionInterface #-}
{-# OPTIONS -XFlexibleInstances #-}
{-# OPTIONS -XTypeSynonymInstances #-}

{-|
Module      : Csound
Description : Haskell foreign function interface to Csound, including a playCsound function for Euterpea.
Copyright   : (c) Michael Gogins, 2019.
License     : LGPL-v2.1
Maintainer  : michael.gogins@gmail.com
Stability   : experimental
Portability : POSIX

This module implements a dynamic FFI binding to the core of the Csound API on
POSIX systems. The module includes a playCsound function for rendering 
Euterpea Music values in a separate thread, using Csound with an embedded 
Csound orchestra.

This module assumes that all binaries, including ghc and Csound, are built for 
64 bit CPU architecture.

These functions have exactly the same names, and exactly the same semantics, as 
the corresponding functions in the Csound API. See 
<https://raw.githubusercontent.com/csound/csound/develop/include/csound.h csound.h> 
or the API documentation at https://csound.com/docs/api/index.html for more 
information. Only some of the Csound API is exposed to Haskell.
-}

module Csound (libCsound, 
    csoundCompileCsdText,
    csoundCreate,
    csoundDestroy,
    csoundPerform,
    csoundReadScore,
    csoundSetControlChannel,
    csoundSetOption,
    csoundStart,
    csoundStop,
    toIStatement,
    performCsdTextThreaded,
    toCsound,
    playCsound,
    csd
    ) where
import Control.Concurrent
import Control.DeepSeq
import Control.Exception
-- import Control.Parallel 
import Data.Typeable
import Foreign
import Foreign.C.String
import Foreign.C.Types
import Foreign.Ptr
import System.IO.Unsafe
import System.Info
import System.Posix.DynamicLinker
import System.Random
import Text.Printf
import Text.RawString.QQ
import Euterpea
import Euterpea.Music

{-
Sample CSD String that can be used to test or demonstrate the API.
-}
csd :: String
csd = [r|
<CsoundSynthesizer>
<CsOptions>
; Select audio/midi flags here according to platform
; Audio out   Audio in    No messages
-odac -m195 -d
; For Non-realtime ouput leave only the line below:
; -o madsr.wav -W ;;; for file output any platform
</CsOptions>
<CsInstruments>

/* Written by Michael Gogins */
; Initialize the global variables.
sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 10000000

; Connect up the instruments to create a signal flow graph.

connect "SimpleSine",   "leftout",     "Reverberator",     	"leftin"
connect "SimpleSine",   "rightout",    "Reverberator",     	"rightin"

connect "Moogy",        "leftout",     "Reverberator",     	"leftin"
connect "Moogy",        "rightout",    "Reverberator",     	"rightin"

connect "Reverberator", "leftout",     "Compressor",       	"leftin"
connect "Reverberator", "rightout",    "Compressor",       	"rightin"

connect "Compressor",   "leftout",     "Soundfile",       	"leftin"
connect "Compressor",   "rightout",    "Soundfile",       	"rightin"

; Turn on the "effect" units in the signal flow graph.

alwayson "Reverberator", 0.8, 12000
alwayson "Compressor"
alwayson "Soundfile"

instr SimpleSine, 9
  ihz = cpsmidinn(p4)
  iamplitude = ampdb(p5)
  print ihz, iamplitude
  ; Use ftgenonce instead of ftgen, ftgentmp, or f statement.
  isine ftgenonce 0, 0, 4096, 10, 1
  a1 oscili iamplitude, ihz, isine
  aenv madsr 0.05, 0.1, 0.5, 0.2
  asignal = a1 * aenv
  ; Stereo audio outlet to be routed in the orchestra header.
  outleta "leftout", asignal * 0.25
  outleta "rightout", asignal * 0.75
endin

instr Moogy, 33
  ihz = cpsmidinn(p4)
  iamplitude = ampdb(p5)
  ; Use ftgenonce instead of ftgen, ftgentmp, or f statement.
  isine ftgenonce 0, 0, 4096, 10, 1
  asignal vco iamplitude, ihz, 1, 0.5, isine
  kfco line 200, p3, 2000
  krez init 0.9
  asignal moogvcf asignal, kfco, krez, 100000
  ; Stereo audio outlet to be routed in the orchestra header.
  outleta "leftout", asignal * 0.75
  outleta "rightout", asignal * 0.25
endin

instr Reverberator
  ; Stereo input.
  aleftin inleta "leftin"
  arightin inleta "rightin"
  idelay = p4
  icutoff = p5
  aleftout, arightout reverbsc aleftin, arightin, idelay, icutoff
  ; Stereo output.
  outleta "leftout", aleftout
  outleta "rightout", arightout 
endin

instr Compressor
  ; Stereo input.
  aleftin inleta "leftin"
  arightin inleta "rightin"
  kthreshold = 25000
  icomp1 = 0.5
  icomp2 = 0.763
  irtime = 0.1
  iftime = 0.1
  aleftout dam aleftin, kthreshold, icomp1, icomp2, irtime, iftime
  arightout dam arightin, kthreshold, icomp1, icomp2, irtime, iftime
  ; Stereo output.
  outleta "leftout", aleftout 
  outleta "rightout", arightout 
endin

instr Soundfile
  ; Stereo input.
  aleftin inleta "leftin"
  arightin inleta "rightin"
  outs aleftin, arightin
endin

</CsInstruments>
<CsScore>
; Run for 6 minutes.
f 0 360
</CsScore>
</CsoundSynthesizer>
|]

{-|
Handle to the Csound shared library, which is assumed to be built for 64 bit 
CPU architecture, and is dynamically loaded here.
-}
libCsound :: DL
libCsound = unsafePerformIO $ dlopen "libcsound64.so" [RTLD_LAZY, RTLD_GLOBAL]

csoundCreateAddress = unsafePerformIO $ dlsym libCsound "csoundCreate"
type CsoundCreate = Word64 -> IO Word64
foreign import ccall unsafe "dynamic" csoundCreateWrapper :: (FunPtr CsoundCreate) -> CsoundCreate
{-|
Creates a new instance of Csound and returns a handle to it. The first 
parameter may be a pointer or handle to user data, which Csound will store 
and pass to callbacks.
-}
csoundCreate :: Word64     -- ^ User data, a pointer or handle, may be 0.
             -> IO Word64  -- ^ A new instance of csound.
csoundCreate = csoundCreateWrapper csoundCreateAddress

csoundCompileCsdTextAddress = unsafePerformIO $ dlsym libCsound "csoundCompileCsdText"
type CsoundCompileCsdTextC = Word64 -> CString -> IO Word64
foreign import ccall unsafe "dynamic" csoundCompileCsdTextWrapper :: (FunPtr CsoundCompileCsdTextC) -> CsoundCompileCsdTextC
csoundCompileCsdTextC :: CsoundCompileCsdTextC
csoundCompileCsdTextC = csoundCompileCsdTextWrapper csoundCompileCsdTextAddress
{-|
Compiles a String containing Csound code in the format of a CSD file, and 
returns the result.
-}
csoundCompileCsdText :: Word64      -- ^ An instance of Csound.
                     -> String      -- ^ Csound orchestra, in the format of a Csound Structured Data file. 
                                    --   The \<CsScore\> element can be empty, except for an "f 0 \<duration\>" 
                                    --   statement to ensure Csound performs for the intended duration. 
                     -> IO Word64   -- ^ 0 for success, or an error code for failure.
csoundCompileCsdText csound csd = withCString csd $ \ccsd -> csoundCompileCsdTextC csound ccsd

csoundPerformAddress = unsafePerformIO $ dlsym libCsound "csoundPerform"
type CsoundPerform = Word64 -> IO Word64
foreign import ccall unsafe "dynamic" csoundPerformWrapper :: (FunPtr CsoundPerform) -> CsoundPerform
{-|
After an orchestra has been compiled and csoundStart has been called, 
actually runs the Csound performance, which may be infinite or finite in 
duration.
-}
csoundPerform :: Word64    -- ^ An instance of Csound. Before calling this function,
                           --   the orchestra must be compiled (e.g. with csoundCompileCsdText)
                           --   and csoundStart must be called.
              -> IO Word64 -- ^ 0 for success, or an error code for failure.
csoundPerform = csoundPerformWrapper csoundPerformAddress

csoundStartAddress = unsafePerformIO $ dlsym libCsound "csoundStart"
type CsoundStart = Word64 -> IO Word64
foreign import ccall unsafe "dynamic" csoundStartWrapper :: (FunPtr CsoundStart) -> CsoundStart
{-|
After a Csound orchestra has been compiled, opens input and output devices and files 
and otherwise initializes all resources required for the actual performance.
-}
csoundStart :: Word64    -- ^ An instance of Csound.
            -> IO Word64 -- ^ 0 for success, or an error code for failure.
csoundStart = csoundStartWrapper csoundStartAddress

csoundSetOptionAddress = unsafePerformIO $ dlsym libCsound "csoundSetOption"
type CsoundSetOptionC = Word64 -> CString -> IO Word64
foreign import ccall unsafe "dynamic" csoundSetOptionWrapper :: (FunPtr CsoundSetOptionC) -> CsoundSetOptionC
csoundSetOptionC :: CsoundSetOptionC
csoundSetOptionC = csoundSetOptionWrapper csoundSetOptionAddress
{-|
Sets a Csound option, before starting a performance. The option may include 
a flag and a value, but may not include any spaces, e.g. "-ofilename" not 
"-o filename".
-}
csoundSetOption :: Word64    -- ^ An instance of Csound. This function should be called before 
                             --   calling csoundStart.
                -> String    -- ^ An option and its value, if any, in a single token without spaces.
                -> IO Word64 -- ^ 0 for success, or an error code for failure.
csoundSetOption csound option = withCString option $ \coption -> csoundSetOptionC csound coption

csoundStopAddress = unsafePerformIO $ dlsym libCsound "csoundStop"
type CsoundStop = Word64 -> IO Word64
foreign import ccall unsafe "dynamic" csoundStopWrapper :: (FunPtr CsoundStop) -> CsoundStop
{-|
Stops an ongoing Csound performance.
-}
csoundStop :: Word64    -- ^ An instance of Csound.
           -> IO Word64 -- ^ 0 for success, or an error code for failure.
csoundStop = csoundStopWrapper csoundStopAddress

csoundReadScoreAddress = unsafePerformIO $ dlsym libCsound "csoundReadScore"
type CsoundReadScoreC = Word64 -> CString -> IO Word64
foreign import ccall unsafe "dynamic" csoundReadScoreWrapper :: (FunPtr CsoundReadScoreC) -> CsoundReadScoreC
csoundReadScoreC :: CsoundReadScoreC
csoundReadScoreC = csoundReadScoreWrapper csoundReadScoreAddress
{-|
Sends a Csound score event or events, in the format of one or more lines of a 
Csound score file, to an ongoing Csound performance.
-}
csoundReadScore :: Word64    -- ^ An instance of Csound.
                -> String    -- ^ One or more lines, separated by '\\n', of Csound "i" statements.
                             --   Score expressions are not permitted. p2, the time of performance, 
                             --   is counted from the time this function is called.
                -> IO Word64 -- ^ 0 for success, or an error code for failure.
csoundReadScore csound score = withCString score $ \cscore -> csoundReadScoreC csound cscore

csoundSetControlChannelAddress = unsafePerformIO $ dlsym libCsound "csoundSetControlChannel"
type CsoundSetControlChannelC = Word64 -> CString -> Double -> IO ()
foreign import ccall unsafe "dynamic" csoundSetControlChannelWrapper :: (FunPtr CsoundSetControlChannelC) -> CsoundSetControlChannelC
csoundSetControlChannelC :: CsoundSetControlChannelC
csoundSetControlChannelC = csoundSetControlChannelWrapper csoundSetControlChannelAddress
{-|
Sends a Csound k-rate control value to the indicated Csound control channel during an 
ongoing Csound performance.
-}
csoundSetControlChannel :: Word64 -> String -> Double -> IO ()
csoundSetControlChannel csound name value = withCString name $ \cname -> csoundSetControlChannelC csound cname value

csoundDestroyAddress = unsafePerformIO $ dlsym libCsound "csoundDestroy"
type CsoundDestroy = Word64 -> IO Word64
foreign import ccall unsafe "dynamic" csoundDestroyWrapper :: (FunPtr CsoundDestroy) -> CsoundDestroy
{-|
Destroys an instance of Csound.
-}
csoundDestroy :: Word64    -- ^ An instance of csound.
              -> IO Word64 -- ^ 0 for success, or an error code for failture.
csoundDestroy = csoundDestroyWrapper csoundDestroyAddress

{-- 
First we define a subset of the Csound API for Haskell. The functions that we 
need to call are more or less the same as the ones in csound.node. We will 
pass only an opaque Csound pointer, ints, doubles, and strings to Csound. 
Some strings will be multi-line literals, which can be implemented in Haskell 
using quasiquotes.

Usage: Load the Csound shared library, create an instance of Csound, 
and pass that instance to all other Csound functions.

See https://stackoverflow.com/questions/25093052/calling-a-c-function-in-haskell-comparing-to-python
https://www.schoolofhaskell.com/user/icelandj/dynamically-link-against-library-in-ghci
https://stackoverflow.com/questions/997738/haskell-ffi-calling-funptrs
https://rosettacode.org/wiki/Call_a_function_in_a_shared_library#Haskell
https://stackoverflow.com/questions/15203847/calling-a-c-function-that-is-referenced-by-a-c-pointer-to-that-function-with-has

csoundCleanup
csoundCompileCsd
csoundCompileCsdText
csoundCompileOrc
csoundCreate
csoundDestroy
csoundEvalCode
csoundGetControlChannel
csoundGetKsmps
csoundGetMetadata
csoundGetNchnls
csoundGetScoreTime
csoundGetVersion
csoundGetSr
csoundInputMessage
csoundIsScorePending
csoundMessage
csoundPerform
csoundPerformAndPostProcess
csoundReadScore
csoundReset
csoundRewindScore
csoundScoreEvent
csoundSetControlChannel
csoundSetMessageCallback
csoundSetMetadata
csoundSetOption
csoundSetOutput
csoundSetScorePending
csoundStart
csoundStop

Next we create a Csound play function for Euterpea.

--}

{-|
Performs a Csound orchestra, as a String in the format of a CSD file, in a 
separate thread. The performance may run in real time or render a soundfile
off-line, and may be infinite or finite in duration. The CSD must be 
configured to run Csound for the required duration. This function creates a 
new instance of Csound and, with it, compiles the CSD text before starting the 
performance thread, so that once this function returns, it is safe to call 
csoundReadScore, csoundSetControlChannelValue, or other performance-time 
functions.
-}
performCsdTextThreaded :: String    -- ^ Contains a Csound orchestra in the format 
                                    --   of a Csound Structured Data file.
                       -> Word64    -- ^ User data in the form of a handle or pointer,
                                    --   which may be 0. Csound will in turn pass this 
                                    --   to certain Csound callbacks.
                                    
                       -> IO Word64 -- ^ A new instance of Csound, which has been used 
                                    --   to compile the orchestra and start the Csound
                                    --   performance.
performCsdTextThreaded csd userdata = do 
    printf "Calling thread:" 
    myThreadId 
    csound <- csoundCreate userdata 
    printf "csound: %d\n" csound 
    compiled <- csoundCompileCsdText csound csd 
    printf "csoundCompileCsdText: %d\n" compiled 
    started <- csoundStart csound 
    printf "csoundStart: %d\n" started 
    thread <- forkIO $ do 
        printf "Csound thread:" 
        myThreadId 
        printf "libCsound:" 
        print libCsound 
        result <- csoundPerform csound 
        printf "csoundPerform: %d\n" result 
        csoundStop csound 
        printf "csoundStop\n" 
        csoundDestroy csound 
        printf "csoundDestroy\n" 
    printf "Done.\n"
    return csound

{-|
Translates a Euterpea MIDI event to a Csound score event String ("i 
statement") with pfields MIDI channel + 1, onset in seconds, duration in 
seconds, MIDI key, and MIDI velocity.
-}
toIStatement :: MEvent -- ^ Euterpea MEvent (MIDI Event).
             -> String -- ^ Csound "i" statement.
toIStatement e = printf "i %3d %9.4f %9.4f %3d %3d 0 0\n" 
    ((toGM $ eInst e) + 1)
    (fromRational $ eTime e) 
    (fromRational $ eDur e) 
    (ePitch e) 
    (eVol e)
    
{-|
Sends a Euterpea MIDI event to an instance of Csound that is performing in a 
separate thread.
-}
toCsound :: Word64    -- ^ Actively performing instance of Csound.
         -> MEvent    -- ^ Euterpea MIDI event.
         -> IO Word64 -- ^ 0 for success, or an error code for failure.
toCsound csound mevent = csoundReadScore csound (toIStatement mevent)

{-|
Performs a Euterpea Music value in a separate thread, using Csound with a 
Csound orchestra passed as a String in the format of a Csound CSD file. The 
user data handle or pointer, which may have a value of 0, is passed to Csound,
when then passes that user data to Csound callbacks. The performance may be 
infinite or finite in duration, and may occur in real time or off-line.
-}
playCsound :: (Show a, ToMusic1 a, Control.DeepSeq.NFData a) => Music a 
                                                                -> String      -- ^ Csound orchestra in the format of a Csound 
                                                                               --   Structured Data File.
                                                                -> Word64      -- ^ User data (handle or pointer) to pass to Csound.
                                                                -> IO [Word64] -- ^ List of results from sending MEvents to Csound.
playCsound m csd userdata = do 
    csound <- performCsdTextThreaded csd userdata
    results <- csound `deepseq` (mapM (\mevent -> (toCsound csound mevent)) (perform m))
    return results
    