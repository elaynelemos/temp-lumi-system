/*
 * Universidade Federal do Vale do São Francisco - Univasf
 * Disciplina:	Sistemas Microcontrolados
 * Discentes:	Anizio Filho, anisiobfilho@gmail.com
				Brendo Cavalcante, brendo.araujo@gmail.com
				Elayne Lemos, elayne.l.lemos@gmail.com
				Ezau Tertuliano, ezaut@hotmail.com
				João Henrique, joaohnick@gmail.com
				Rafael Klebson, rafa_kleb10@hotmail.com
 * Orientador: Prof. Dr. Jadsonlee Silva
 * Nome do projeto no Atmel: sistema_temp_lumi.c
 * Implementação:	Sistema para aferição de temperatura e luminosidade capaz de retornar os
					valores no ambiente formatados via serial. Foram utilizados para o presente
					trabalho o AVR Atmega328p da Atmel, um sensor LM35 e um sensor LDR. 
 * Created: 31/01/2020 09:41:11	
 */ 


#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#define SENSOR_TEMP 0x00 //Define pino equivalente ADCx utilizado para o sensor de temperatura.
#define SENSOR_LUMI 0x05 //Define pino equivalente ADCx utilizado para o sensor de luminosidade.


uint8_t datal, datah, check=0;
float tensao;
uint16_t data;

//Protótipo das funções.
void adcTempSetup();
void adcLumiSetup();
void adcLeitura();
void usartSetup();
void transfer(uint8_t);
void transferString(char*);
void transferNumb(int);
void transferTemp();
void transferLumi();
void serialPrint(uint8_t);

int main(void){
	
	usartSetup();
	
    while (1){
		switch(check){
			
			//Caso base no período do sistema, conta 10s e imprime temperatura, depois
			//conta 5s e imprime luminosidade (passados totais 15s).
			case 0:
				_delay_ms(10000);
				serialPrint(SENSOR_TEMP);
				_delay_ms(5000);
				serialPrint(SENSOR_LUMI);
				check = 1; break;
			
			//Conta o tempo que falta para o ciclo da temperatura e a imprime.
			case 1:
				_delay_ms(5000);
				serialPrint(SENSOR_TEMP);
				check = 2; break;
			
			//Imprime temperatura e luminosidade quando o tempo é mútiplo dos dois
			//períodos (período total do sistema). Após, por exemplo: 30s, 60s, 90s...
			case 2: 
				_delay_ms(10000);
				serialPrint(SENSOR_TEMP);
				serialPrint(SENSOR_LUMI);
				check = 0; break;
			
			//Imprime mensagem de erro caso check não seja corretamente definido.
			default:
				serialPrint(10);
		}
		
   }

}


void adcTempSetup(){
	ADMUX = SENSOR_TEMP; //Chaveia o conversor para o sensor de temperatura.
	ADCSRA = 0x87; //Habilita ADC e define prescaler de 128.
	ADCSRB = 0x00;
	DIDR0 = 0x01;
}

void adcLumiSetup(){
	ADMUX = SENSOR_LUMI; //Chaveia o conversor para o sensor de luminosidade.
	ADCSRA = 0x87; //Habilita ADC e define prescaler de 128.
	ADCSRB = 0x00; 
	DIDR0 = 0x20;
}

void adcLeitura(){
	ADCSRA = (ADCSRA)|(0x40);		//Inicia a conversão.
	while((ADCSRA&0x10)!= 0x10){}	//Termina conversão e verifica flag de finalização ao sair.
	datal = ADCL;					//Guarda os bits menos significativos de ADC.
	datah = ADCH;					//Guarda os bits mais significativos de ADC.
	ADCSRA = ADCSRA | 0x10;			//Zera a flag de finalização.
	data=(datah<<8|datal);			//data recebe o valor completo de ADC.
}


void usartSetup(){
	
	UBRR0H = 0x00; //define taxa de transmissão de 115200bps.
	UBRR0L = 0x08;
	
	UCSR0A = 0x40; //Reseta UDR0.
	UCSR0B = 0x08; //Habilita transmissor UART.
	UCSR0C = 0x06; //Define banda de 8 bits, sem paridade e 1 bit de stop.
}

void transfer(uint8_t byte){
	while((UCSR0A & 0x20)!=0x20){} //Sai do loop quando se encerra a transferência.
	UDR0 = byte;
}

void transferString(char* texto){
	int i;
	for(i=0; texto[i]!='\0'; i++){	//Enquanto não se chega ao final do texto,
		transfer(texto[i]);			//faz a transferência caracter a caracter.
	}
}

void transferNumb(int valor){
	char texto[20];
	int i=18;
	do{
		texto[i] = ((char)valor%10)+'0';	//Extrai unidade e soma a 48 (correspondente a 0 em ASCII)
		valor/=10;							//para normalizar o algarismo.
		i--;
	}while(valor && (i>=2));
	texto[19] = 0;
	
	transferString(texto+i+1);				//Envia o valor pela serial.
}


void transferTemp(){
	
	adcTempSetup();	//Seta ADC para temperatura.
	adcLeitura();	//Faz a leitura da temperatura.
	
	tensao = 4.88*data;	//Ajusta escala de tensão a partir da referência: Resolução = dV/(2^10-1).


	transferNumb(tensao/10);	//Imprime parte inteira da temperatura.
	transferString(",");
	transferNumb((int)tensao%10);	//Imprime a parte decimal.
	transferString(" °C \n");		
}

void transferLumi(){
	
	adcLumiSetup();	//Seta ADC para luminosidade.
	adcLeitura();	//Faz a leitura da luminosodade.
	
	tensao = (1024-(data*0.9765625));	//Ajusta escala de tensão a partir da referência: Resolução = dV/(2^10-1).
	
	transferNumb(tensao/10);	//Imprime parte inteira da luminosidade.
	transferString(",");
	transferNumb((int)tensao%10);	//Imprime a parte decimal.
	transferString("% \n");
	
}

	
void serialPrint(uint8_t sensor){
		if(sensor==SENSOR_TEMP){
			transferTemp(); //Envia pela serial o valor formatado da temperatura.
		}
		else if(sensor==SENSOR_LUMI){
			transferLumi(); //Envia pela serial o valor formatado da luminosidade.
		}
		else{
			transferString("ERROR"); //Envia pela serial a mensagem "ERROR" caso seja passado como parâmetro o pino errado.
		}
	
}