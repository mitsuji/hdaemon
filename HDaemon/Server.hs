module HDaemon.Server where

import Control.Concurrent
import System.IO

data SStatus = Run | Shutdown | Stop deriving( Eq )
type SHandle = MVar SStatus

start :: String -> IO(SHandle)
start confPath = do 
  logHandle <- openFile confPath AppendMode
  hPutStrLn logHandle "start!"
  hFlush logHandle
  handle <- newMVar Run
  forkIO $ serve handle logHandle
  return(handle)
  where
    serve :: SHandle -> Handle  -> IO()  
    serve handle logHandle = do
      status <- readMVar handle
      if status == Run 
        then do
        hPutStrLn logHandle "go!"
        hFlush logHandle
        threadDelay $ 2 * 1000 * 1000
        serve handle logHandle
        else do
        hPutStrLn logHandle "stop!"        
        hFlush logHandle
        hClose logHandle
        modifyMVar_ handle (\_ -> return(Stop))
        
    
stop :: SHandle -> IO()    
stop handle = do 
  modifyMVar_ handle (\_ -> return(Shutdown))
  wait handle
  where
    wait :: SHandle -> IO()
    wait handle = do
      status <- readMVar handle
      if status == Shutdown
        then do
        threadDelay $ 100 * 1000          
        wait handle
        else do
        return()
          
  
 