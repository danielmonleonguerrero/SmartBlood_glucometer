#include<TimerOne.h>
#include<SoftwareSerial.h>
SoftwareSerial BTConnection(10, 11); //RX/TX

//Variables procesar valor glucosa//
float AVglucosa[10];
float Vglucosa = 0;
float Vgm = 0;
float V2 = 0;
int glucosa = 0;
int PinGlucosa = 0;

//Variables gestion interrupciones//
boolean START = false;
int contInt=0;
boolean timerON = false;

//Variables calculo tiempo//
unsigned long tBegin, tEnd;

//CONFIGURACIÓN//
void setup() {
  Timer1.initialize(200000); //Interrupción a los 200ms
  Timer1.attachInterrupt(ISR_Timer); //Interrupcion llama al metodo ISR_Timer
  Timer1.stop();
  attachInterrupt(1, START_MEASURE, FALLING); //Interrupcion llama al metodo START_MEASURE
  pinMode(Vglucosa, INPUT);
  BTConnection.begin(9600);
  Serial.begin(9600);
  Serial.println("Encienda glucometro"); //Mensaje inicial
}

//INTERRUPCIÓN TIMER 200MS//
void ISR_Timer() {
  contInt++;  //Se cuenta las veces que pasan 200ms
  Serial.print("conInt: "); Serial.println(contInt);
}

//INTERRUPCIÓN PULSADOR//
void START_MEASURE() {
  START = true;
  Serial.println("START TRUE");
}

//DETECCIÓN DE PICO TENSIÓN Y TIMER 2 SEGUNDOS//
void loop() {
  if (START == true) {
    if(contInt==0){
    //Tensión antes de aplicar sangre
    Vglucosa = analogRead(PinGlucosa);
    Vglucosa = (Vglucosa / 1023 * 5);
    Serial.print("Tension actual: ");
    Serial.println(Vglucosa);
    }
    
    //Se detecta pico de tension
    if (Vglucosa > 2) {
      if(timerON==false) {
        contInt=0;
        Timer1.start(); //Timer encendido
        tBegin=millis();
        timerON=true;
      }
      if (contInt==11) {//Han pasado 2 segundos
        Timer1.stop();
        tEnd=millis();
        contInt=0;
        noInterrupts(); //Se deshabilita interrupciones para calculo glucosa
        calcularglucosa();
      }
    }
  }
}

//METODO CALCULO VALOR GLUCOSA//
void calcularglucosa() {
  //Calculo tension//
  for (int i = 0; i < 10; i++) {
    Vglucosa = analogRead(PinGlucosa);
    AVglucosa[i] = (Vglucosa / 1023 * 5);     
    Serial.println(AVglucosa[i]);
    Vgm=Vgm+AVglucosa[i];
  }

  //Calculo tiempo//
  Serial.print("Tiempo: ");
  Serial.println(tEnd - tBegin);

  //Calculo valor medio tensión//
  Vgm = Vgm / 10;
  Serial.print("V: "); 
  Serial.println(Vgm);

  //Calculo glucosa//
  V2=Vgm*Vgm;
  glucosa = -48.16*V2 +353.5*Vgm -369.4;
  Serial.print("Valor glucosa: ");
  Serial.println(glucosa);
  BTConnection.print(glucosa, DEC);

  //Reset variables//
  V2=0;
  glucosa = 0;
  Vgm = 0;
  START = false;
  timerON = false;
  interrupts();
  Timer1.stop();
  Serial.println("Encienda glucometro");
}


