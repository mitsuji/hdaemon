module HDaemon.Daemon where

import Control.Concurrent
import System.Environment
import System.Posix.Signals
import Foreign
import HDaemon.Server


termHandler :: MVar Bool -> SHandle -> IO()
termHandler continue handle = do 
  stop handle 
  modifyMVar_ continue (\_ -> return(False))

hupHandler :: IO()
hupHandler = return()


foreignMain :: IO()
foreignMain = do
  confPath <- getConfPath
  handle <- start confPath
  continue <- newMVar True
  installHandler sigTERM (Catch $ termHandler continue handle ) Nothing
  installHandler sigHUP (Catch hupHandler) Nothing
  wait continue
  where
    wait :: MVar Bool -> IO() 
    wait continue = do
      continue' <- readMVar continue
      if continue'
        then do
        threadDelay $ 100 * 1000
        wait continue 
        else do
        return()
    
    getConfPath :: IO(String)
    getConfPath = do
      args <- getArgs
      return(args !! 0)
        

foreign export ccall foreignMain :: IO ()