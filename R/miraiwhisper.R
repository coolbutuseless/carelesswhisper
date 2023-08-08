
#' Real Time Audio Transcription using Whisper
#' 
#' @param seconds a recording of this length is sampled repeatedly.
#' 
#' @return Invisible NULL. The transcribed text is output to the console.
#' 
#' @details Send an interrupt to break and exit from the function.
#' 
#' @export
#' 
whisper_rt <- function(seconds = 5L) {
  
  ctx <- whisper_init()
  m1 <- mirai(carelesswhisper::record_audio(seconds), seconds = seconds)
  
  repeat {
    call_mirai(m1)
    m2 <- mirai(carelesswhisper::record_audio(seconds), seconds = seconds)
    res <- carelesswhisper::whisper(ctx = ctx, snd = m1[["data"]])
    cat(res)
    call_mirai(m2)
    m1 <- mirai(carelesswhisper::record_audio(seconds), seconds = seconds)
    res <- carelesswhisper::whisper(ctx = ctx, snd = m2[["data"]])
    cat(res)
  }
  
}
