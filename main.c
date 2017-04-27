//
//
// Created by: Victor Herrera-Ramirez and Brendan Currie
// under the direction of Dr. Shankar as a 'final project'.
//
// Date: 4-22-2017
// Version 1.03
//
// This program will take the inputs of a joystick and
// map them to a 3 x 3 matrix of leds. The leds will light
// on and off depending on the direction of joystick.
//
// TO-DO: add in 'simon says' game function, as described
// within the  Breadboard Set-Up section.



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//               HARDWARE DOCUMENTATION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//               Keyes_SJoys Set-up

//   _______________________
//  |      Keyes_SJoys      |
//  |                       |
//  |                       |
//  |                       |
//  |                       |
//  |                       |
//  |                       |
//  |                       |
//  |_______________________|
//  |     |     |     |     |
// GND   Vcc  X_ax   Y_ax  press
//                        [unused]

// GND is connected to the breadboard's ground terminal, and Vcc is
// connected to the breadboard's positive terminal.

// For correct orientation, we use the words "Keyes_SJoys" to mean
// the "top", as shown in the above diagram.
//
// We used a red wire for the x-axis, connected directly to the MSP_430 as
// shown later on below and we used a grey wire for the y-axis, also
// connected directly to the MSP_430.
//
// The press function of the joystick is unused in this project.
//
// Since we will be using the ADC with 10 bits of precision, the maximum
// analog value we can obtain is 1024. So we divide by 3 to obtain 341.
//
// Any value less than 341 will register as low, any higher than 682 (2 * (1024/3)),
// will register as high, and any value in-between these two extremes will register
// as center.


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//                   MSP_430 SETUP

// For the following diagram, the values within the [] represent
// the column number followed by the row number of the 3 by 3 array of
// leds.
//
// The reason the x_axis is second, instead of first, is because the ADC
// updates starting from the highest pin (normally 1.7, but in this case 1.1),
// and updates the lowest pin (1.0) last.
//
// * Note these are wired up to the whole development board, not the stand-alone chip *
//                 ___________________
//     [UNUSED]  -|     MSP_430       |-  [UNUSED]
// [ADC Y_AXIS]  -|      WIRING       |-  [UNUSED]
// [ADC X_AXIS]  -|     DIAGRAM       |-  [UNUSED]
//        [1,1]  -|                   |-  [UNUSED]
//        [1,2]  -|                   |-  [UNUSED]
//        [1,3]  -|                   |-  [2,2]
//        [2,1]  -|                   |-  [2,3]
//        [3,1]  -|                   |-  [UNUSED]
//        [3,2]  -|                   |-  [FAIL LED]
//        [3,3]  -|___________________|-  [PASS LED]



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//                   Breadboard Set-Up
//
// On the breadboard, we set up a 3x3 matrix of leds, with
// the negative leg of the leds wired up to the MSP_430 based
// on the above diagram (in other words, they are directly connected).
//
// The positive leg of the leds is wired up to a 330 ohm resistor,
// and then connected to 3.5 volts (postive terminal).
//
// The Joystick set-up has already been discussed in an above section.
//
// We also have wired up two addition leds, both connected directly to the
// MSP_430 via their negative leg, and connected in series with a 330 ohm
// resistor to the positive terminal.
//
// These two additional leds will be used in a future updated for a simple
// "simon says", (follow the light) game. The 'pass' led will count as an
// additional point on correctly following the light. While the 'fail' led
// will light on a failed attempt.



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// START OF CODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <msp430.h>

//////////////////////////////////////////////////////
//globals and defines
/////////////////////////////////////////////////////

// We used these toggles to turn off all the leds,
// except for the one the user is currently on.

#define TopLeftToggle       (P1OUT^=BIT2)
#define TopMiddleToggle     (P1OUT^=BIT5)
#define TopRightToggle      (P2OUT ^=BIT0)

#define MiddleLeftToggle    (P1OUT ^= BIT3)
#define CenterToggle        (P1OUT ^= BIT6)
#define MiddleRightToggle   (P2OUT ^= BIT1)

#define BottomLeftToggle    (P1OUT ^= BIT4)
#define BottomMiddleToggle  (P1OUT ^= BIT7)
#define BottomRightToggle   (P2OUT ^= BIT2)


// these are to store the values of the x and y
// axes.

int x_axis= 0;
int y_axis  = 0;

// integer value for the ADCMEM to write into
int ADCReading [2];

//////////////////////////////////////////////////////
// end of globals and defines
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////
// start of functions
/////////////////////////////////////////////////////

void ConfigureIO (void) {

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer

    // The follow hex value (0xFD) turns on pins 1.2 through
    // 1.7 into output for the leds.
    // Pins 1.0 and 1.1 are kept at 0 for the axes input.
    P1DIR = 0xFD;
    P1OUT = 0xFD;

    // we will only use pins 2.0 through 2.2
    P2DIR = 0x07;
    P2OUT = 0x07;

} // end of function

/**/

void ConfigureADC(void) {

   // The following will turn on the ADC and open
   // the information channels for pins 1.0 and 1.1
   // it will then sample at 16 cycles from the reference
   // value of ground and Vcc of 3.5.
   ADC10CTL1 = INCH_1 | CONSEQ_1;
   ADC10CTL0 = ADC10SHT_2 | MSC | ADC10ON;
   while (ADC10CTL1 & BUSY);
   ADC10DTC1 = 0x03;
   ADC10AE0 |= (BIT0 | BIT1 );

} // end of function

/**/

void getAnalogValues() {

    // The following will read in the values from the
    // now open ADC (ADCMEM) into our array 'ADCReading[]'
    // we will put the *second* value into the x-axis, and the
    // *first* value into the y-axis, since the ADC reads from
    // high pin values, to low pin values. This will make
    // the x-axis the "first" input.
    y_axis = 0; x_axis= 0;

    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);
    ADC10SA = (unsigned)&ADCReading[0];
    ADC10CTL0 |= (ENC | ADC10SC);
    while (ADC10CTL1 & BUSY);

    x_axis  = ADCReading[1];
    y_axis= ADCReading[0];

} // end of function

/**/

void clear () {

    // This will give all the pins the same value
    // so we can then toggle them off depending
    // on which direction the joystick is pointing.
    P1OUT = 0x00000000;
    P2OUT = 0x00000000;

} // end of function

/**/

void gameLogic(){


    // The following code replicates a lot of code, so each
    // if statement basically will check if the joystick is
    // pointed in its quadrant. If it is, it will toggle off all
    // other leds, and wait until the joystick moves out of its
    // quadrant.
    while (1) {

            if (x_axis <= 341 && y_axis >= 682) {
                // x_axis is 'low', and y_axis is 'high'
                // this is the top left corner
                clear();

                TopMiddleToggle;
                TopRightToggle;
                MiddleLeftToggle;
                CenterToggle;
                MiddleRightToggle;
                BottomLeftToggle;
                BottomMiddleToggle;
                BottomRightToggle;


                while(x_axis <= 341 && y_axis >= 682) {

                    getAnalogValues();
                }

            } // END OF THE IF STATEMENT

            else if (x_axis >= 341 && x_axis <= 682 && y_axis >= 682) {
                // x_axis is 'center', and y_axis is 'high'
                // this is the top center
                clear();

                TopLeftToggle;
                TopRightToggle;
                MiddleLeftToggle;
                CenterToggle;
                MiddleRightToggle;
                BottomLeftToggle;
                BottomMiddleToggle;
                BottomRightToggle;

                while (x_axis >= 341 && x_axis <= 682 && y_axis >= 682){

                    getAnalogValues();
                }
            } // END OF THE ELSE IF STATEMENT

            else if (x_axis >= 682 && y_axis >= 682){
                // x_axis is 'high', and y_axis is 'high'
                // this is the top right
                clear();

                TopLeftToggle;
                TopMiddleToggle;
                MiddleLeftToggle;
                CenterToggle;
                MiddleRightToggle;
                BottomLeftToggle;
                BottomMiddleToggle;
                BottomRightToggle;

                while (x_axis >= 682 && y_axis >= 682){
                    getAnalogValues();
                }
            } // END OF ELSE IF FUNCTION



            else if (x_axis <= 341 && y_axis >= 341 && y_axis <= 682) {
                 // x_axis is 'low', and y_axis is 'center'
                 // this is the left center
                 clear();

                 TopLeftToggle;
                 TopMiddleToggle;
                 TopRightToggle;
                 CenterToggle;
                 MiddleRightToggle;
                 BottomLeftToggle;
                 BottomMiddleToggle;
                 BottomRightToggle;

                 while (x_axis <= 341 && y_axis >= 341 && y_axis <= 682){

                    getAnalogValues();
                 }
            } // END OF THE ELSE IF STATEMENT

            else if (x_axis >= 682 && y_axis >= 341 && y_axis <= 682) {
                 // x_axis is 'high', and y_axis is 'center'
                 // this is the right center
                 clear();

                 TopLeftToggle;
                 TopMiddleToggle;
                 TopRightToggle;
                 MiddleLeftToggle;
                 CenterToggle;
                 BottomLeftToggle;
                 BottomMiddleToggle;
                 BottomRightToggle;

                 while (x_axis >= 682 && y_axis >= 341 && y_axis <= 682){

                   getAnalogValues();
                }
            } // END OF THE ELSE IF STATEMENT


            else if (x_axis <= 341 && y_axis <= 341) {
                 // x_axis is 'low', and y_axis is 'low'
                 // this is the bottom left
                 clear();

                 TopLeftToggle;
                 TopMiddleToggle;
                 TopRightToggle;
                 MiddleLeftToggle;
                 CenterToggle;
                 MiddleRightToggle;
                 BottomMiddleToggle;
                 BottomRightToggle;

                      while (x_axis <= 341 && y_axis <= 341){

                          getAnalogValues();
                     }
            } // END OF THE ELSE IF STATEMENT



            else if (x_axis >= 341 && x_axis <= 682 && y_axis <= 341) {
                 // x_axis is 'center', and y_axis is 'low'
                 // this is the bottom center
                 clear();

                 TopLeftToggle;
                 TopMiddleToggle;
                 TopRightToggle;
                 MiddleLeftToggle;
                 CenterToggle;
                 MiddleRightToggle;
                 BottomLeftToggle;
                 BottomRightToggle;

                      while (x_axis >= 341 && x_axis <= 682 && y_axis <= 341){

                          getAnalogValues();
                     }
            } // END OF THE ELSE IF STATEMENT



            else if (x_axis >= 682 && y_axis <= 341) {
                 // x_axis is 'high', and y_axis is 'low'
                 // this is the bottom right
                 clear();

                 TopLeftToggle;
                 TopMiddleToggle;
                 TopRightToggle;
                 MiddleLeftToggle;
                 CenterToggle;
                 MiddleRightToggle;
                 BottomLeftToggle;
                 BottomMiddleToggle;

                      while (x_axis >= 682 && y_axis <= 341){

                          getAnalogValues();
                     }
            } // END OF THE ELSE IF STATEMENT



            else  {
                clear();

                TopLeftToggle;
                TopMiddleToggle;
                TopRightToggle;
                MiddleLeftToggle;
                MiddleRightToggle;
                BottomLeftToggle;
                BottomMiddleToggle;
                BottomRightToggle;

                while (x_axis >= 341 && x_axis <= 682 && y_axis >= 341 && y_axis <= 682) {
                    getAnalogValues();
                } // END OF VOLTAGE WHILE LOOP
            } // END OF THE ELSE STATEMENT


        }// END OF THE INFINITE WHILE LOOP
} // END OF FUNCTION

//////////////////////////////////////////////////////
// end of functions
/////////////////////////////////////////////////////



//////////////////////////////////////////////////////
// Start of Main function
/////////////////////////////////////////////////////
void main(void) {

   ConfigureIO();
   ConfigureADC();
   getAnalogValues();
   gameLogic();

} // END OF THE MAIN FUNCTION

//////////////////////////////////////////////////////
// End of Main function
/////////////////////////////////////////////////////


//////////////////////////////////////////////////////
// Interrupt code
/////////////////////////////////////////////////////

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
__bic_SR_register_on_exit(CPUOFF);
}
