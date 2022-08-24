// Monitoramento de Carga e Descarga de uma bateria selada

#include "mbed.h"
#include "ssd1306.h"


AnalogIn tensao(A0);
AnalogIn corrente (A1);
DigitalOut led(D2);
//Serial pc(USBTX, US BRX);
float v_lido ; // Variável que recebe o valor de tensão
float a_lido ; // Variável que recebe o valor de corrent






SSD1306 lcd (D14, D15); // assumes default I2C address of 0x78



int main()
{


//***********************Iniciaçização Display********************************//

    lcd.speed (SSD1306::Medium);  // set working frequency
    lcd.init();
    lcd.set_contrast(200);
    lcd.cls();                    // clear frame buffer
    lcd.locate (1,0);                 // set text cursor to line 3, column 1
    lcd.printf (" MONITOR BATERIA"); // print to frame buffer
    
//*********************Calculo Divisor AD8210*********************************//
    while(1) {
        v_lido= tensao;
        a_lido = corrente;
        v_lido = (v_lido*4.0)*3.3; // O valor 4.0 é realação entre a tensão de entrada (12V) com a tensão que foi calculada no divisor de tensão que é 3V
        a_lido =(a_lido*3.3)/1.375; // O valor de 1.375 é a razão(resistência) entre a tensão Vshunt (3.6 V) com a corrente do circuito (2.4 A)
        
//******************************************************************************




        lcd.locate (3,0);
        lcd.printf ("VOLTS: %1.2f",v_lido); // print to frame buffer
        
        lcd.locate (5,0);
        lcd.printf ("Corrente: %1.2f",a_lido); // print to frame buffer
        
        wait(0.5);
        
        lcd.redraw();
        }
       
}


