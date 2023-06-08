


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Record audio from the default input device
#' 
#' @param seconds recording length
#' 
#' @return Numeric vector of mono sound data sampled at 16kHz
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
record_audio <- function(seconds) {
  .Call(record_audio_, seconds)
}


whisper_params <- list(
  n_threads        = 4, # number of threads
  translate        = FALSE, # translate from source language to english
  language         = "en"
)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Named list of two-letter language codes to use as \code{language} parameter
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
whisper_lang_codes <- list(
  "english"        = "en",
  "chinese"        = "zh",
  "german"         = "de",
  "spanish"        = "es",
  "russian"        = "ru",
  "korean"         = "ko",
  "french"         = "fr",
  "japanese"       = "ja",
  "portuguese"     = "pt",
  "turkish"        = "tr",
  "polish"         = "pl",
  "catalan"        = "ca",
  "dutch"          = "nl",
  "arabic"         = "ar",
  "swedish"        = "sv",
  "italian"        = "it",
  "indonesian"     = "id",
  "hindi"          = "hi",
  "finnish"        = "fi",
  "vietnamese"     = "vi",
  "hebrew"         = "he",
  "ukrainian"      = "uk",
  "greek"          = "el",
  "malay"          = "ms",
  "czech"          = "cs",
  "romanian"       = "ro",
  "danish"         = "da",
  "hungarian"      = "hu",
  "tamil"          = "ta",
  "norwegian"      = "no",
  "thai"           = "th",
  "urdu"           = "ur",
  "croatian"       = "hr",
  "bulgarian"      = "bg",
  "lithuanian"     = "lt",
  "latin"          = "la",
  "maori"          = "mi",
  "malayalam"      = "ml",
  "welsh"          = "cy",
  "slovak"         = "sk",
  "telugu"         = "te",
  "persian"        = "fa",
  "latvian"        = "lv",
  "bengali"        = "bn",
  "serbian"        = "sr",
  "azerbaijani"    = "az",
  "slovenian"      = "sl",
  "kannada"        = "kn",
  "estonian"       = "et",
  "macedonian"     = "mk",
  "breton"         = "br",
  "basque"         = "eu",
  "icelandic"      = "is",
  "armenian"       = "hy",
  "nepali"         = "ne",
  "mongolian"      = "mn",
  "bosnian"        = "bs",
  "kazakh"         = "kk",
  "albanian"       = "sq",
  "swahili"        = "sw",
  "galician"       = "gl",
  "marathi"        = "mr",
  "punjabi"        = "pa",
  "sinhala"        = "si",
  "khmer"          = "km",
  "shona"          = "sn",
  "yoruba"         = "yo",
  "somali"         = "so",
  "afrikaans"      = "af",
  "occitan"        = "oc",
  "georgian"       = "ka",
  "belarusian"     = "be",
  "tajik"          = "tg",
  "sindhi"         = "sd",
  "gujarati"       = "gu",
  "amharic"        = "am",
  "yiddish"        = "yi",
  "lao"            = "lo",
  "uzbek"          = "uz",
  "faroese"        = "fo",
  "haitian creole" = "ht",
  "pashto"         = "ps",
  "turkmen"        = "tk",
  "nynorsk"        = "nn",
  "maltese"        = "mt",
  "sanskrit"       = "sa",
  "luxembourgish"  = "lb",
  "myanmar"        = "my",
  "tibetan"        = "bo",
  "tagalog"        = "tl",
  "malagasy"       = "mg",
  "assamese"       = "as",
  "tatar"          = "tt",
  "hawaiian"       = "ha",
  "lingala"        = "ln",
  "hausa"          = "ha",
  "bashkir"        = "ba",
  "javanese"       = "jw",
  "sundanese"      = "su"
)




#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Fetch a copy of the default whisper parameters
#' 
#' \describe{
#'    \item{n_threads}{Number of threads to use when processing. Default: 4}
#'    \item{translate}{Translate from source language into english? Default: FALSE}
#'    \item{language}{language represented in audio.  Use 'auto' to automatically
#'          detect language. Default: 'en'}
#' }
#' 
#' @return Named list of default parameters
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
whisper_default_params <- function() {
  whisper_params
}




#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Initialise whisper by loading a model
#' 
#' @param model_path path to whisper.cpp model. By default
#'        this will use the "ggml-tiny.bin" file included with this 
#'        package installation, which is a tiny multi-language model 
#'        See README for this package, or the original 
#'        whisper.cpp documentation, for how to download other models.
#' @param verbose Be verbose about model initialisation?  Logical. Default: FALSE
#' 
#' @return whisper context (\code{ctx})
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
whisper_init <- function(model_path = system.file("ggml-tiny.bin", package = "carelesswhisper", mustWork = TRUE), 
                         verbose = FALSE) {
  .Call(whisper_init_, model_path, isTRUE(verbose))
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Perform automatic speech recognition of the given sound sample
#' 
#' @param snd Sound data.  16kHz mono audio in a numeric vector 
#'        with all values in the range [-1, 1].  This package includes the function
#'         `record_audio()` which will record audio in this format.
#'         You could also use \code{audio::record()} or any other audio package
#'         you have access to.
#' @param ctx whisper context (which you have previously created using \code{whisper_init()})
#' @param params parameters for whisper. A user should usually create a default set 
#'       of parameters by calling
#'        \code{whisper_param_defaults()} and then modify. 
#' @param verbose logical. be verbose? default: FALSE.
#' 
#' @examples
#' \dontrun{
#'   ctx <- whisper_init()  # Initialise the model
#'   snd <- record_audio(2) # record 2 seconds of audio 
#'   whisper(ctx, snd)      # perform speech recognition
#' }
#' 
#' @return Character string
#' 
#' @importFrom utils modifyList
#' @export
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
whisper <- function(ctx, snd, params = list(), verbose = FALSE) {
  
  # Sanitize params
  params <- modifyList(whisper_params, params, keep.null = TRUE)
  params <- params[names(params) %in% names(whisper_params)]
  params$detect_language = ifelse(params$language == 'auto', TRUE, FALSE)
  
  if (verbose) {
    print(params)
  }
  
  trimws(.Call(whisper_, ctx, snd, params))
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# inline testing
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if (FALSE) {
  ctx <- whisper_init()
  snd <- record_audio(2)
  whisper(ctx, snd)
  
  snd <- record_audio(2)
  whisper(ctx, snd)
  
  ctx_tiny  <- whisper_init()
  ctx_small <- whisper_init("/Users/mike/projectsdata/whisper.cpp/ggml-small.en.bin")
  
  snd <- record_audio(5)
  whisper(ctx_tiny , snd)
  whisper(ctx_small, snd)
  
  
  
  ctx_tiny  <- whisper_init()
  ctx_small <- whisper_init("/Users/mike/projectsdata/whisper.cpp/ggml-small.bin", list(n_threads = 10))
  snd <- record_audio(2)
  whisper(ctx_tiny , snd, params = list(translate = FALSE, language = 'ja'))
  whisper(ctx_tiny , snd, params = list(translate =  TRUE, language = 'ja'))
  
  whisper(ctx_small, snd, params = list(translate = FALSE, language = 'ja'))
  whisper(ctx_small, snd, params = list(translate =  TRUE, language = 'ja'))
  
  whisper(ctx_tiny , snd)
  whisper(ctx_small, snd)
}



