module HDaemon.Server where

import Control.Concurrent
import System.IO

data SHandle = SHandle { continue::MVar Bool
                       , shutdown::MVar()
                       }

start :: String -> IO(SHandle) 
start confPath = do
  logHandle <- openFile confPath AppendMode
  hPutStrLnFlush logHandle "start!"
  continue' <- newMVar True
  shutdown' <- newEmptyMVar
  let handle = SHandle { continue = continue'
                       , shutdown = shutdown' 
                       }
  forkIO $ serve handle logHandle
  return(handle)
  where
    
    serve :: SHandle -> Handle  -> IO()  
    serve handle logHandle = do
      continue' <- readMVar $ continue handle
      if continue' == True 
        then do
          hPutStrLnFlush logHandle "go!"
          threadDelay $ 2 * 1000 * 1000
          serve handle logHandle
        else do 
          hPutStrLnFlush logHandle "stop!"        
          hClose logHandle
          putMVar (shutdown handle) () 
    

    hPutStrLnFlush :: Handle -> String -> IO()
    hPutStrLnFlush handle value = do
      hPutStrLn handle value
      hFlush handle


stop :: SHandle -> IO()    
stop handle = do 
  continue' <- modifyMVar (continue handle) (\x -> return(False, x))
  if continue' == True
    then do
    takeMVar $ shutdown handle 
    return()
    else do
    return()
      
      
