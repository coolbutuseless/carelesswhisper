
<!-- README.md is generated from README.Rmd. Please edit that file -->

# carelesswhisper <img src="man/figures/logo.png" align="right" height="230/"/>

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
<!-- badges: end -->

`carelesswhisper` is a (dependency-free!) package for recording audio
and then performing Automatic Speech Recognition (ASR) using
[`whisper.cpp`](https://github.com/ggerganov/whisper.cpp).

This package includes the smallest multi-language `whisper.cpp` model
(70MB) and it is possible to record audio and perform speech recognition
immediately after install (without chasing down any file downloads or
dependencies).

## What’s in the box

- `whisper_init()` and `whisper()` for interfacing with the built-in
  [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp) code for
  speech recognition
- `record_audio()` will record audio from your default input device
  using the built-in [`miniaudio`](https://github.com/mackron/miniaudio)
  C library.
- `whisper_lang_codes` is the list of 2-letter language codes understood
  by whisper.
- `jfk` a short audio sample for testing

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/carelesswhisper) with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/carelesswhisper')
```

### Platform notes:

The audio recording in this package uses
[`miniaudio`](https://github.com/mackron/miniaudio) - this is
cross-platform and works on macOS, Windows and Linux.

No attempt has been made to choose good parameters for the compiler - so
it is not optimized for any particular platform. This probably leaves a
lot of speed on the table.

## Using `carelesswhisper`

``` r
library(carelesswhisper)

# Initialise whisper with built-in model (tiny, multi-language)
ctx <- whisper_init()

# Record 2 seconds of audio and perform speech recognition
snd <- record_audio(2)
whisper(ctx, snd)

# Record 2 seconds of audio and perform speech recognition
# Tell whisper it should treat the audio as spoken Japanese
snd <- record_audio(2)
whisper(ctx, snd, params = list(language = 'ja'))

# Ask whisper to translate the Japanese into English
whisper(ctx, snd, params = list(language = 'ja', translate = TRUE))
```

``` r
# Detailed results using the included test sample of JFK
# audio::play(jfk)
ctx <- whisper_init()

whisper(ctx, jfk)
#> [1] "And so my fellow Americans ask not what your country can do for you ask what you can do for your country."

whisper(ctx, jfk, details = TRUE)
#>    lang_id segment_idx start  end token_idx token_id      token       prob
#> 1        0           0     0 1050         0    50364    [_BEG_] 0.83815217
#> 2        0           0     0 1050         1      400        And 0.75576884
#> 3        0           0     0 1050         2      370         so 0.92633528
#> 4        0           0     0 1050         3      452         my 0.66328335
#> 5        0           0     0 1050         4     7177     fellow 0.99870670
#> 6        0           0     0 1050         5     6280  Americans 0.96553099
#> 7        0           0     0 1050         6     1029        ask 0.46481884
#> 8        0           0     0 1050         7      406        not 0.82426906
#> 9        0           0     0 1050         8      437       what 0.70673782
#> 10       0           0     0 1050         9      428       your 0.94040984
#> 11       0           0     0 1050        10     1941    country 0.98648512
#> 12       0           0     0 1050        11      393        can 0.98244804
#> 13       0           0     0 1050        12      360         do 0.99388093
#> 14       0           0     0 1050        13      337        for 0.97501081
#> 15       0           0     0 1050        14      291        you 0.99160498
#> 16       0           0     0 1050        15     1029        ask 0.30006781
#> 17       0           0     0 1050        16      437       what 0.83009559
#> 18       0           0     0 1050        17      291        you 0.97196573
#> 19       0           0     0 1050        18      393        can 0.96942919
#> 20       0           0     0 1050        19      360         do 0.94574499
#> 21       0           0     0 1050        20      337        for 0.96720922
#> 22       0           0     0 1050        21      428       your 0.94377720
#> 23       0           0     0 1050        22     1941    country 0.98983622
#> 24       0           0     0 1050        23       13          . 0.52694887
#> 25       0           0     0 1050        24    50889  [_TT_525] 0.04029942
```

## Using different models

The model included with this package (and used by default when calling
`whisper_init()`) is the smallest, multi-language model:
`ggml-tiny.bin`.

**Larger models** exist, but they need more RAM and run slower.

**English-only models** can perform better if English is the only
language you expect to encounter.

If you want to use any of the different/more complex models, just
download them and give the path to `whisper_init(path_to_model)`

These models can be downloaded from:

- [huggingface](https://huggingface.co/ggerganov/whisper.cpp/tree/main)
- [ggerganov](https://ggml.ggerganov.com/)

## Future

The code is MIT licensed. Feel free to fork this and make of it what you
want.

Pull requests also welcomed - especially if they’re about fixing any
cross-platform issues.

## Related Software

- [bnosac](https://www.bnosac.be/) also has an R package which wraps
  `whisper.cpp` -
  [audio.whisper](https://github.com/bnosac/audio.whisper)

## Licenses

- This R package is MIT licensed. See file: LICENSE
- The included [`miniaudio`](https://github.com/mackron/miniaudio)
  library is MIT licensed. See file `LICENSE-miniaudio.txt`
- The included [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp)
  code is MIT licensed. See file `LICENSE-whisper.cpp.txt`
  - Code: <https://github.com/ggerganov/whisper.cpp>
  - Commit: 041be06d58
  - 20 May 2023
  - Modifications for R compatibility
    - replaced all “fprintf(stderr, )” with “Rprintf()”
    - replaced all “printf()” with “Rprintf()”
    - replaced “fprintf() + abort()” with “error()”
    - commented out all the benchmarking code (which include some puts()
      and rand() calls and is not used in this pkg)

## Acknowledgements

- R Core for developing and maintaining the language.
- CRAN maintainers, for patiently shepherding packages onto CRAN and
  maintaining the repository
