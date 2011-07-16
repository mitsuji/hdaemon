module HDaemon.Foreign where

import System.Environment
import Foreign
import HDaemon.Server


foreignStart :: IO(StablePtr(SHandle))
foreignStart = do
  logPath <- getLogPath
  handle <- start logPath
  ptr <- newStablePtr handle
  return ptr
  where
    getLogPath :: IO(String)
    getLogPath = do
      args <- getArgs
      return(args !! 0)
  
foreignStop :: StablePtr(SHandle) -> IO()  
foreignStop ptr = do   
  handle <- deRefStablePtr ptr
  stop handle
  freeStablePtr ptr


foreign export stdcall foreignStart :: IO (StablePtr (SHandle))
foreign export stdcall foreignStop :: StablePtr (SHandle) -> IO()


  
