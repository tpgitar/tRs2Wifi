#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"




//--------------------------------------------------------------
extern void configure_led(void);
extern void task_Led_500ms(void *pvParameters);
extern void setLedGreen();
extern void setLedBlue();
extern void setLedRed();
extern void initWifi(void);
extern void configureUart();

extern void tcp_server_task(void *pvParameters);

//--------------------------------------------------------------

void app_main(void)
{
	
	configure_led();
	
	configureUart();

	
	xTaskCreate(task_Led_500ms,        // funkcja taska
                "task_Led",      // nazwa taska
                1024,              // rozmiar stosu
                NULL,              // parametry
                5,                 // priorytet
                NULL);             // uchwyt (opcjonalnie)
	
	
	initWifi();
	

   xTaskCreate(tcp_server_task,
               "tcp_server",
               8192,
               NULL,
               5,
               NULL);	



	
    while (true) 
    {
	
		//blink_led();
		
	/*	
		ucCnt++;
		if(ucCnt > 3)
		{ucCnt = 0;}
		
		switch(ucCnt)
		{
			case 0:
			{
				setLedRed();
			}break;

			case 1:
			{
				setLedGreen();
			}break;
			
			case 2:
			{
				setLedBlue();
			}break;

			
		}*/
		
		
		
   //     printf("1  Hello from app_main!\n");
        
        
        
        
        
        sleep(1);
    }
}
//--------------------------------------------------------------