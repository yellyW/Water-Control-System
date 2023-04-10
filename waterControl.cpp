#include "mbed.h"
#include "TextLCD.h"
#include "DRV8825.h"
#include <stdio.h>

DigitalOut myled2(LED2);
TextLCD lcd(p22, p21, p23,p24,p25,p26);
double currentFreshPumpPosition = 0.0;
double currentSaltPumpPosition = 0.0;
bool runMotorIsUsed = false;
DigitalOut buzz = DigitalOut (p30); 
int suckFresh = 0;
int pushFreshCounter = 0;
int suckSalt = 0;
int pushSaltCounter = 0;
int flaskVolume = 400;
int flaskMax = 700;
float vIn;


DigitalOut myledTwo (p5);
DigitalOut myled1 (p6);
DigitalIn switch4 (p10);
DigitalIn switch3 (p9);
DigitalIn switch2 (p8);
DigitalIn switch1 (p7);

float getSalinity(float vOut)
{
   //return 25.226583*vOut - 2.448955;
   //return 29.627014*vOut - 8.014815; // the 40.9
   //return 22.233147*vOut - 2.89452; // when it was working on tuesday
   return 53.511482*vOut - 9.502664; 
}

float getTemperature(float vOut)
{
    float rt = (vOut*-15000) / (vOut - vIn);
    float kelvinTemp =  1 / (0.00102119+0.000222468*log(rt)+0.000000133342*pow(log(rt),3));
    return kelvinTemp - 273.15;
}

float getTemperatureTwo(float vOut)
{
    return -33.917197*vOut + 90.010709;
}

void runMotors(int motorNb)
{
    // motor 1 : salty water - motor 2: fresh water
    
    //motorNb = 2;
    
    int maxSpeed =  6450;
    int microstepsPerStep = 16;
    double* currentPumpPosition = &currentSaltPumpPosition;
    DRV8825 stpr_mtr1 = DRV8825(p21, p27, p28, p29, p22, p23);
    DRV8825 stpr_mtr2 = DRV8825(p16, p15, p14, p13, p12, p11);
    DRV8825* stpr_mtr = &stpr_mtr1;
    DigitalOut enbl = DigitalOut(p21);
    int direction = 1;
    int* suck = &suckSalt;
    int* pushCounter = &pushSaltCounter;
   

    if (motorNb == 2)
    {
        stpr_mtr = &stpr_mtr2;
        enbl = DigitalOut(p16);
        currentPumpPosition = &currentFreshPumpPosition;
        direction = 0;
        suck = &suckFresh;
        pushCounter = &pushFreshCounter;
        
    }
        
        if (*suck == 0)
        {
            for (int z = 0; z < 30*2; z++)
            {
                for (int i = 500; i <= maxSpeed; i+=5) 
                {
                    stpr_mtr->settings(1/microstepsPerStep,abs(0-direction), i);
                } 
                for (int i = maxSpeed ; i >= 500; i-=5) 
                {
                    stpr_mtr->settings(1/microstepsPerStep,abs(0-direction), i);
                } 
            }
            *suck = 1;
            *pushCounter = 0;
            *currentPumpPosition = 32.7;
            wait(15);   
        }
        else
        {
            *pushCounter += 1;
            for (int z = 0; z < 10*2; z++)
            {
            for (int i = 500; i <= maxSpeed; i+=5) 
                {
                    stpr_mtr->settings(1/microstepsPerStep,abs(1-direction), i);
                } 
                for (int i = maxSpeed; i >= 500; i-=5) 
                {
                    stpr_mtr->settings(1/microstepsPerStep,abs(1-direction), i);
                } 
            }
            if (*pushCounter == 3)
            {
                *suck = 0;
            }
            flaskVolume+= 10;
            *currentPumpPosition = *currentPumpPosition - (10-0.367);
            if (*currentPumpPosition <= 6)
            {
                wait(10);
            }
        } 
}

void resetSyringes()
{
    int maxSpeed =  6450;
    int microstepsPerStep = 16;
    DRV8825 stpr_mtr1 = DRV8825(p21, p27, p28, p29, p22, p23);
    DRV8825 stpr_mtr2 = DRV8825(p16, p15, p14, p13, p12, p11);
        float switchThreeValue = switch3;
        if (switchThreeValue*3.3 < 1.5 )
        {
            for (int z = 0; z < 2; z++)
            {
                for (int i = 500; i <= maxSpeed; i+=5) 
                {
                    stpr_mtr2.settings(1/microstepsPerStep,1, i);
                } 
                for (int i = maxSpeed; i >= 500; i-=5) 
                {
                    stpr_mtr2.settings(1/microstepsPerStep,1, i);
                }
            } 
            flaskVolume++;
            suckFresh = 0;
            pushFreshCounter =0;
            currentFreshPumpPosition = 0.0;
        }    
        float switchFourValue = switch4; 
        if(switchFourValue*3.3 < 1.5)
        {
            for (int z = 0; z < 2; z++)
            {
                for (int i = 500; i <= maxSpeed; i+=5) 
                {
                     stpr_mtr1.settings(1/microstepsPerStep,0, i);
                } 
                for (int i = maxSpeed; i >= 500; i-=5) 
                {
                    stpr_mtr1.settings(1/microstepsPerStep,0, i);
                } 
            }
            flaskVolume++;
            suckSalt = 0;
            pushSaltCounter =0;
            currentSaltPumpPosition = 0.0;
        }   
}

void lowerHigherSalinity(float sal)
{
    float salinity = sal;
    float lowerSal = salinity - 2.5;
    float higherSal = salinity + 2.5;
    float salMiddle = salinity;
    
    
    float switchTwoValue = switch2; 
    if (flaskVolume < flaskMax - 10)
    { 
           float switchThreeValue = switch3;
           float switchFourValue = switch4;
          
           if (switchThreeValue*3.3 > 1.5 && switchFourValue*3.3 > 1.5)
           {
                lcd.printf("Sal: %.2f",salMiddle);
                wait(0.5);
                lcd.cls();
            }
        
        if (switchTwoValue*3.3 > 2.7)
        {
            if (lowerSal >= 20.0 && higherSal <= 30.0)
            {
                myledTwo.write(1.0f);
            }
            else if (higherSal > 30.0)
            {
                myledTwo.write(0.0f);
                if (!runMotorIsUsed)
                {
                    runMotorIsUsed = true;
                    runMotors(2);
                    wait(3);
                    runMotorIsUsed = false;
                }
            }
            else if (lowerSal < 20.0)
            {
                myledTwo.write(0.0f);
                if (!runMotorIsUsed)
                {
                    runMotorIsUsed = true;
                    runMotors(1);
                    wait(3);
                    runMotorIsUsed = false;
                }
             }
        }
        else
        {
             myledTwo.write(0.0f);
            if (!runMotorIsUsed)
            {
                runMotorIsUsed = true;
                resetSyringes();
                runMotorIsUsed = false;
            }
        }
    }
    else
    {
        buzz.write(1.0f);
        wait(10);
        buzz.write(0.0f);
        wait(10);
    }
}
void controlHeat(float tempVout, int time)
{
    float vLower = tempVout - 0.04;
    float vHigher = tempVout + 0.04;
    float tempHigher = getTemperature(vLower) ;//- 1 ;
    float tempLower = getTemperature(vHigher) ;//- 2  ;
    float tempMiddle = getTemperature(tempVout);
    
    AnalogOut AinNi (p18); 
    float fNi = AinNi; // or just float f = Ain; reads the digital output
    float VinNi = fNi *3.3;
    
     AnalogIn Ainn (p19);
        float salinityVOut = Ainn;
        salinityVOut *= 3.3;
        salinityVOut += 0.01;

     if (tempLower < 35)
    {
        AinNi.write(1.0f);
        myled1.write(1.0f); 
        
        lcd.printf("Temp: %f",tempMiddle);
        wait(0.5);
        lcd.cls(); 
    }
    else 
    {
        AinNi.write(0.0f); 
        myled1.write(0.0f);
        lowerHigherSalinity(getSalinity(salinityVOut));
    }
}
int main() 
{
    //lcd.printf("Hello world/n/r");
    buzz.write(0.0f);
    int time = 0;
    while (1==1)
    {
        myled2 = 0;
        AnalogIn Ain (p20); // configures pin20 for analog input. Creates an object Ain.
        float f = Ain; // or just float f = Ain; reads the digital output
        vIn = f *3.3;
    
        AnalogIn Aini (p17);
        float tempVOut = Aini;
        tempVOut *= 3.3;
    
        float switchOneValue = switch1;
        float switchTwoValue = switch2; 
        if (switchOneValue*3.3 < 1.5 && switchTwoValue*3.3 < 1.5 && vIn > 3.0)
        {
            if (!runMotorIsUsed)
            {
                runMotorIsUsed = true;
                resetSyringes();
                runMotorIsUsed = false;
            }
        }
        else if(switchOneValue*3.3 > 1.5)
        {
            controlHeat(tempVOut, time);
        }
        else
        {
            myled1.write(0.0f);
        }
    
        time += 1;
        if (time == 1000000)
        {
             time = 0;
        }
    }
}



