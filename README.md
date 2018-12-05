# Spock

A Speck-like block cipher that hopes to improve on overall security by working with 32 bit words and using two parallel Speck round functions.  In addition to the 64 bits of round key applied to each round is a 128 bit diffusion key that is added mod 2^32 each round.

More rounds and extra keys means more resources and slightly slower performance than Speck.

Rounds = 40

Key size = 128 bit (currently implemented for 128 bit keys.  Other key sizes will follow)
