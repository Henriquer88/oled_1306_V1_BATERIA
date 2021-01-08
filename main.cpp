#include "mbed.h"
//#include "USBSerial.h"
#include "ssd1306.h"
#include "SDFileSystem.h"

#define fator_filtro 5
#define ctoT 0.224f
 
SDFileSystem sd(D11, D12, D13, D10, "sd"); // the pinout on the mbed Cool Components workshop board

FILE *myLogFile;
  
AnalogIn amp_pin(A0);
AnalogIn volt_pin(A3);

//Virtual serial port over USB
//USBSerial device;
Serial pc(USBTX, USBRX);
 
SSD1306 lcd (D14, D15); // assumes default I2C address of 0x78

Ticker flipper;
Timer fileOpenTimer;
Timer debug_timer;

//DigitalOut led(D13);
DigitalIn sw(PC_13);

float mAmp_S;
float mAmp;
float mAmp_filtred;
float mAmp_filtred_old;

int second = 0;
int minute = 0;
int hour = 0;
bool flag;
 
FILE *nextLogFile(void)
{
    static unsigned int fileNumber = 0;
    char fileName[32];
    FILE *filePtr = NULL;
    do 
    {
        if (filePtr != NULL)
            fclose(filePtr);
        sprintf(fileName,"/sd/log%04u.csv",fileNumber++);
        filePtr = fopen(fileName,"r");
    } while (filePtr != NULL);
    return fopen( fileName,"w");
} 
 
 
void mAmp_time_base()
{
    mAmp_S = mAmp_S + mAmp_filtred; 
    //led=!led;
    second++;
    flag =! flag;
} 
 
int main(void) 
{
    
    float volt;
    float volt_filtred;
    float volt_filtred_old;
    
    float mAmp_H;
    bool start = 0;
    
    lcd.speed (SSD1306::Medium);  // set working frequency
    lcd.init();                   // initialize SSD1306
    lcd.set_contrast(100);
    lcd.cls();                    // clear frame buffer
    lcd.locate (3,2);             // set text cursor to line 3, column 1
    lcd.printf ("Starting up"); // print to frame buffer
    for( int i = 0; i < 16; i++)
    {
        lcd.locate (4,i);
        lcd.printf (".");
        lcd.redraw();
        wait_ms(150);
    }
    wait_ms(300);
    lcd.cls();
    lcd.redraw();
        
    myLogFile = nextLogFile();
    if (!myLogFile) {
        // ERROR failed to open the first log file for writing.
        // The SD card is missing, not working, read only or full?
 
        return 1; // probably want to exit the program in this situation
    }
    float fator_filtro_x_ctoT = fator_filtro * ctoT;
    float soma_fator_filtro_x_ctoT = 1.0f + fator_filtro_x_ctoT;
    
    printf("%f\n",fator_filtro_x_ctoT);  
    printf("%f\n",soma_fator_filtro_x_ctoT);  
      
    while(1)
    {     
        //debug_timer.reset();     
        //debug_timer.start();
        
         
        if((sw == 0) && (start == 0))
        {
            wait_ms(200);
            if((sw == 0) && (start == 0))
            {
                start = 1;
                wait_ms(1000);
                flipper.attach(&mAmp_time_base, 1.0f); 
                fileOpenTimer.start();  
                lcd.cls();  
            }
        }
        
        if((sw == 0) && (start == 1))
        {
            wait_ms(200);
            if((sw == 0) && (start == 1))
            {
                start = 0;
                flipper.detach(); 
                lcd.locate(1,1);
                lcd.printf("Log finished");
                lcd.redraw();
                fileOpenTimer.stop(); 
                fileOpenTimer.reset(); // restart the timer 
                fclose(myLogFile); // close the current file
                myLogFile = nextLogFile(); // open a new file   
                wait_ms(5000);
                mAmp_S = 0;
                mAmp_H = 0;
                second = 0;
                minute = 0;
                hour = 0;  
                lcd.cls();
                lcd.redraw();
            }
        }
        
        
        //mAmp = amp_pin.read() * 3.3f * 14.6f;
        //mAmp = mAmp * 1.37f * 100.0f;
        
        mAmp = amp_pin.read() * 4870.0f;//6600.66f ;
        
        mAmp_filtred =  ((fator_filtro_x_ctoT * mAmp) + (mAmp_filtred_old)) / soma_fator_filtro_x_ctoT;
        mAmp_filtred_old = mAmp_filtred;
        
        
        volt = volt_pin.read() * 36.45f; //3.3f * divisor resistivo 10k / 1k
         
        volt_filtred =  ((fator_filtro_x_ctoT * volt) + (volt_filtred_old)) / soma_fator_filtro_x_ctoT;
        volt_filtred_old = volt_filtred; 
         
         
        if(start == 0)
        {
            lcd.locate(1,1);
            lcd.printf("Press to log"); 
        }
        
        if(start == 1)
        {
            mAmp_H = mAmp_S / 3600.0f;  
            lcd.locate(1,1);
            lcd.printf("Logging..."); 
            lcd.locate(5,1);
            lcd.printf("%8.2f mAh",mAmp_H); 
            lcd.locate(7,1);
            lcd.printf("%02i:%02i:%02i",hour, minute, second); 
        }
             
             
        if(second > 59)
        {
            second = 0;
            minute++;
        }
        
        if(minute > 59)
        {
            minute = 0;
            hour++; 
        }
        
        if(hour > 23)
        {
            hour = 0; 
        }
        
        lcd.locate(3,1);
        lcd.printf("%8.2f V",volt_filtred);
        
        lcd.locate(4,1);
        lcd.printf("%8.2f mA",mAmp_filtred); 
          
        lcd.redraw();
        
        if(flag == 1)
        {
            flag = 0;
            if(start == 1)
            {
                fprintf(myLogFile, "%02i:%02i:%02i ; %8.2f ; %8.2f ; %8.2f\n", hour, minute, second, volt_filtred, mAmp_filtred, mAmp_H);
            }
        }
        
        if (fileOpenTimer > (5*60)) 
        { // file has been open 5 minutes
            fclose(myLogFile); // close the current file
            myLogFile = nextLogFile(); // open a new file   
            fileOpenTimer.reset(); // restart the timer
        }
        wait_ms(200);  
        
        //debug_timer.stop();
        //printf("%f\n", debug_timer.read());
    }
}
