// Pines para ATtiny85
const int pinLED = PB1;       // PB1 (Pin físico 6) - Salida de la señal PWM
const int pinFreq = A1;      // PB2 (Pin físico 7 -> Entrada Analógica A1)
const int pinDuty = A2;      // PB4 (Pin físico 3 -> Entrada Analógica A2)

// Variables de control de tiempo
unsigned long tiempoAnterior = 0;
unsigned long tiempoOn = 0;
unsigned long tiempoOff = 0;
bool estadoLED = false;
bool alternarLectura = false; // Para turnar las lecturas analógicas

void setup() {
  pinMode(pinLED, OUTPUT);
  
  // Inicialización para evitar problemas en el primer ciclo
  tiempoOn = 100000;
  tiempoOff = 100000;
}

void loop() {
  // --- OPTIMIZACIÓN: Leemos un potenciómetro a la vez ---
  if (alternarLectura) {
    // 1. Leer y calcular Frecuencia (CAMBIADO: Ahora el mínimo es 5 Hz hasta 140 Hz)
    int lecturaFreq = analogRead(pinFreq);
    int frequencia = map(lecturaFreq, 0, 1023, 3, 140); 
    
    unsigned long periodoTotal = 1000000UL / frequencia;
    
    // Recalcular tiempos basados en el nuevo período
    int lecturaDuty = analogRead(pinDuty);
    tiempoOn = map(lecturaDuty, 0, 1023, 0, periodoTotal);
    tiempoOff = periodoTotal - tiempoOn;
  } else {
    // 2. Leer y calcular Duty Cycle basado en el período actual
    int lecturaDuty = analogRead(pinDuty);
    unsigned long periodoTotal = tiempoOn + tiempoOff;
    if (periodoTotal == 0) periodoTotal = 1000000UL;
    
    tiempoOn = map(lecturaDuty, 0, 1023, 0, periodoTotal);
    tiempoOff = periodoTotal - tiempoOn;
  }
  
  alternarLectura = !alternarLectura; // Alterna el turno en el siguiente loop

  // --- GENERADOR DE ONDA POR SOFTWARE ---
  unsigned long tiempoActual = micros();

  if (estadoLED) {
    // Si está encendido, esperar a que termine tiempoOn
    if (tiempoActual - tiempoAnterior >= tiempoOn) {
      estadoLED = false;
      digitalWrite(pinLED, LOW);
      tiempoAnterior = tiempoActual;
    }
  } else {
    // Si está apagado, esperar a que termine tiempoOff
    if (tiempoActual - tiempoAnterior >= tiempoOff) {
      if (tiempoOn > 0) { // Evita encender si el Duty es 0%
        estadoLED = true;
        digitalWrite(pinLED, HIGH);
      }
      tiempoAnterior = tiempoActual;
    }
  }
}
