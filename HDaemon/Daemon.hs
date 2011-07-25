module HDaemon.Daemon where

import Control.Concurrent
import System.Environment
import System.Posix.Signals
import Foreign
import HDaemon.Server


termHandler :: MVar() -> SHandle -> IO()
termHandler continue handle = do 
  stop handle 
  putMVar continue ()

hupHandler :: IO()
hupHandler = return()


foreignMain :: IO()
foreignMain = do
  confPath <- getConfPath
  handle <- start confPath
  continue <- newEmptyMVar
  installHandler sigTERM (Catch $ termHandler continue handle ) Nothing
  installHandler sigHUP (Catch hupHandler) Nothing
  takeMVar continue
  return()
  where
    getConfPath :: IO(String)
    getConfPath = do
      args <- getArgs
      return(args !! 0)
        

foreign export ccall foreignMain :: IO ()