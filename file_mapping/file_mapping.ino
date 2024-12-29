#ifndef FILEMAPPING_H

#define FILEMAPPING_H



int numberMappings[91] = {};



void initializeNumberMappings() {

 numberMappings[0] = 29;

 numberMappings[1] = 25;

 numberMappings[10] = 76;

 numberMappings[11] = 73;

 numberMappings[12] = 78;

 numberMappings[13] = 79;

 numberMappings[14] = 68;

 numberMappings[15] = 70;

 numberMappings[16] = 65;

 numberMappings[17] = 64;

 numberMappings[18] = 54;

 numberMappings[19] = 55;

 numberMappings[2] = 34;

 numberMappings[20] = 49;

 numberMappings[21] = 48;

 numberMappings[22] = 42;

 numberMappings[23] = 45;

 numberMappings[24] = 53;

 numberMappings[25] = 56;

 numberMappings[26] = 63;

 numberMappings[27] = 60;

 numberMappings[28] = 67;

 numberMappings[29] = 71;

 numberMappings[3] = 38;

 numberMappings[30] = 52;

 numberMappings[31] = 57;

 numberMappings[32] = 62;

 numberMappings[33] = 61;

 numberMappings[34] = 50;

 numberMappings[35] = 47;

 numberMappings[36] = 43;

 numberMappings[37] = 44;

 numberMappings[38] = 75;

 numberMappings[39] = 74;

 numberMappings[4] = 15;

 numberMappings[40] = 10;

 numberMappings[41] = 5;

 numberMappings[42] = 17;

 numberMappings[43] = 18;

 numberMappings[44] = 33;

 numberMappings[45] = 40;

 numberMappings[46] = 27;

 numberMappings[47] = 26;

 numberMappings[48] = 80;

 numberMappings[49] = 86;

 numberMappings[5] = 19;

 numberMappings[50] = 35;

 numberMappings[51] = 39;

 numberMappings[52] = 28;

 numberMappings[53] = 24;

 numberMappings[54] = 8;

 numberMappings[55] = 6;

 numberMappings[56] = 16;

 numberMappings[57] = 20;

 numberMappings[58] = 106;

 numberMappings[59] = 105;

 numberMappings[6] = 9;

 numberMappings[60] = 82;

 numberMappings[61] = 85;

 numberMappings[62] = 91;

 numberMappings[63] = 90;

 numberMappings[64] = 108;

 numberMappings[65] = 103;

 numberMappings[66] = 97;

 numberMappings[67] = 100;

 numberMappings[68] = 11;

 numberMappings[69] = 4;

 numberMappings[7] = 7;

 numberMappings[70] = 107;

 numberMappings[71] = 104;

 numberMappings[72] = 96;

 numberMappings[73] = 101;

 numberMappings[74] = 83;

 numberMappings[75] = 84;

 numberMappings[76] = 93;

 numberMappings[77] = 89;

 numberMappings[78] = 36;

 numberMappings[79] = 37;

 numberMappings[8] = 98;

 numberMappings[80] = 21;

 numberMappings[81] = 14;

 numberMappings[82] = 3;

 numberMappings[83] = 12;

 numberMappings[84] = 23;

 numberMappings[85] = 30;

 numberMappings[86] = 41;

 numberMappings[87] = 32;

 numberMappings[88] = 88;

 numberMappings[89] = 94;

 numberMappings[9] = 99;

 numberMappings[90] = 22;

};





struct SoundMappings {

 uint8_t andSound;

 uint8_t happyCrafting;

 uint8_t minute;

 uint8_t minutes;

 uint8_t necklaceDetected;

 uint8_t necklaceRemoved;

 uint8_t remaining;

 uint8_t second;

 uint8_t seconds;

 uint8_t timerIsNowPaused;

 uint8_t timerPaused;

 uint8_t timerStarted;

 uint8_t timerStopped;

 uint8_t timeToTakeABreak;

 uint8_t totalTime;

 uint8_t youHave;

 uint8_t beep;

 uint8_t timerUp;

};



// const SoundMappings otherSoundMappings;



SoundMappings otherSoundMappings;



void initializeOtherSoundMappings() {

 otherSoundMappings.andSound = 66;

 otherSoundMappings.happyCrafting = 72;

 otherSoundMappings.minute = 69;

 otherSoundMappings.minutes = 13;

 otherSoundMappings.necklaceDetected = 58;

 otherSoundMappings.necklaceRemoved = 95;

 otherSoundMappings.remaining = 46;

 otherSoundMappings.second = 51;

 otherSoundMappings.seconds = 92;

 otherSoundMappings.timerIsNowPaused = 1;

 otherSoundMappings.timerPaused = 59;

 otherSoundMappings.timerStarted = 77;

 otherSoundMappings.timerStopped = 31;

 otherSoundMappings.timeToTakeABreak = 102;

 otherSoundMappings.totalTime = 2;

 otherSoundMappings.youHave = 109;

 otherSoundMappings.beep = 110;

 otherSoundMappings.timerUp = 111;

}



int getNumberSound(int file) {

 return numberMappings[file];

}



#endif