#include <msp430.h>
#include <time.h>
#include <stdlib.h>
#include "peripherals.h"
/**
 * main.c
 */

void sleep(int time) {
    volatile long int i,j;
    for (i = 0; i < time; i++) {
        j = 12;
        while (j > 0) {
            j--;
        }
    }
}

void buzzz(int pitch)
{
    // Initialize PWM output on P3.5, which corresponds to TB0.5
    P3SEL |= BIT5; // Select peripheral output mode for P3.5
    P3DIR |= BIT5;

    TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
    TB0CTL  &= ~TBIE;                       // Explicitly Disable timer interrupts for safety

    // Now configure the timer period, which controls the PWM period
    // Doing this with a hard coded values is NOT the best method
    // We do it here only as an example. You will fix this in Lab 2.
    TB0CCR0   = pitch;                    // Set the PWM period in ACLK ticks
    TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

    // Configure CC register 5, which is connected to our PWM pin TB0.5
    TB0CCTL5  = OUTMOD_7;                   // Set/reset mode for PWM
    TB0CCTL5 &= ~CCIE;                      // Disable capture/compare interrupts
    TB0CCR5   = TB0CCR0/2;                  // Configure a 50% duty cycle
}

void lightNBuzz(unsigned char num) {
    if (num == '1') {
        Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 15, 45, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        setLeds(8);
        buzzz(20);
    } else if (num == '2') {
        Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 35, 45, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        setLeds(4);
        buzzz(60);
    } else if (num == '3') {
        Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 55, 45, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        setLeds(2);
        buzzz(100);
    } else if (num == '4') {
        Graphics_drawStringCentered(&g_sContext, "4", AUTO_STRING_LENGTH, 75, 45, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        setLeds(1);
        buzzz(140);
    } else {
        setLeds(15);
        buzzz(250);
    }
}

void holdSpin() {
    uint8_t input = getKey();
    while(input != 0) {
        input = getKey();
    }
}

int main(void) {
    uint8_t level;
    uint8_t input;
    uint8_t simon[32];
    uint8_t i;
    uint8_t j;
    uint8_t k;
    uint8_t good;

    WDTCTL = WDTPW | WDTHOLD;
    srand(time(NULL));
    initLeds();
    configDisplay();
    configKeypad();
    while (1) { // ON LOOP
        level = 0;
        good = 1;
        for (i = 0; i < 32; i++) {
            simon[i] = rand() % 4;
        }

        Graphics_clearDisplay(&g_sContext);

        Graphics_drawStringCentered(&g_sContext, "ECE 2049", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "SIMON SAYS", AUTO_STRING_LENGTH, 48, 30, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "PRESS * TO PLAY", AUTO_STRING_LENGTH, 48, 45, OPAQUE_TEXT);

        Graphics_flushBuffer(&g_sContext);

        while (1) { // READ START BUTTON
            if (getKey() == '*') {
                break;
            }
        }
        Graphics_clearDisplay(&g_sContext);
        sleep(1000);
        Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        sleep(1000);
        Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48, 25, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        sleep(1000);
        Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48, 35, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        sleep(1000);
        Graphics_drawStringCentered(&g_sContext, "START", AUTO_STRING_LENGTH, 48, 45, OPAQUE_TEXT);
        Graphics_flushBuffer(&g_sContext);
        sleep(1000);
        Graphics_clearDisplay(&g_sContext);

        while(good == 1) { // GAME LOOP
            sleep(2000);
            for (j = 0; j <= level; j++) { // LIGHT LOOP
                lightNBuzz((simon[j] + 0x31));
                sleep(2000 - (level * 50));
                BuzzerOff();
                setLeds(0);
                Graphics_clearDisplay(&g_sContext);
                sleep(2000 - (level * 50));
            }
            for (k = 0; k <= level; k++) { // READ LOOP
                holdSpin();
                input = getKey();
                while (input == 0) {
                    input = getKey();
                }
                lightNBuzz(input);
                holdSpin();
                BuzzerOff();
                setLeds(0);
                Graphics_clearDisplay(&g_sContext);
                if (input != (simon[k] + 0x31)) {
                    lightNBuzz(69);
                    Graphics_drawStringCentered(&g_sContext, "YOU GET NOTHING", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "YOU LOSE", AUTO_STRING_LENGTH, 48, 45, OPAQUE_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "GOOD DAY SIR", AUTO_STRING_LENGTH, 48, 75, OPAQUE_TEXT);
                    Graphics_flushBuffer(&g_sContext);
                    holdSpin();
                    sleep(15000);
                    BuzzerOff();
                    setLeds(0);
                    good = 0;
                    break;
                }
            }
            if ((level == 31) && (good == 1)) {
                Graphics_drawStringCentered(&g_sContext, "YOU WIN!", AUTO_STRING_LENGTH, 48, 45, OPAQUE_TEXT);
                Graphics_drawStringCentered(&g_sContext, "HOW?", AUTO_STRING_LENGTH, 48, 65, OPAQUE_TEXT);
                Graphics_flushBuffer(&g_sContext);
                for (good = 0; good < 20; good++) {
                    lightNBuzz(69);
                    sleep(1000);
                    BuzzerOff();
                    setLeds(0);
                    sleep(1000);
                }
                break;
            }
            level++;
        }
    }
    return 0;
}
