# Countdown

#### Acknowledgements: 
This is a basic Countdown game using [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) as a framework. 
The music is implemented using an adapted version of [olcNoiseMaker](https://github.com/OneLoneCoder/synth/blob/master/olcNoiseMaker.h).
These were both made by Javidx9 AKA OneLoneCoder. Worth checking out his YouTube channel for some interesting videos. The rest of the game is designed by me, although for the calculation of the numbers round I took heavy inspiration from this [article](https://datagenetics.com/blog/august32014/index.html) by [DataGenetics.com](https://datagenetics.com/index.html).

#### The controls are simple:
To start, click the big start button with your mouse. You can enter player names beforehand if you like, but this currently does not affect the way the game is played.

Press C or V to start a letters round. C and V refer to consonant and vowel respectively. A consonant or a vowel will appear, and then you can add more with the C and V keys until there are 9 letters selected. The countdown will start and you'll have 30 seconds to think of an answer. There is currently no solver for this round.

Press B or S to start a numbers round. B and S refer to big and small numbers (the top and the bottom, as per the show). A big or small number will appear, and as before you can select the remaining five numbers. The countdown will start, and at the end a solution will appear with a press of the Enter key.

Press A to start an anagram round. The anagram is currently hardcoded so will need changing between games to keep things fresh. The countdown will start immediately. You can pause the clock with the Space bar and reveal the answer with the Enter key.

In the future the user will enter the names of players and the game will choose each round type automatically. The above controls will remain for debugging purposes.
