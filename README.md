# model-railway-ct-reader

## Hardware

* Arduino UNO/Mega
* ZMTC103C
* 30AWG wire

## Setup

1. Wind the 30AWG wire through the current transformer 8/10 times. More winds means more current is available for the CT to detect. For smaller scales, the current draw can be very small, so more winds results in more accurate reads.
2. Connect the ZMTC103C Ground to an Arduino ground, and the signal to an analog pin.
3. Update the code so that the INPUT_CHANNEL value is the analog input.